/**
 * I have a high interest in all things related to video game programming and production.
 * One of the hot topics in this respect is data oriented design, that aims at laying out
 * your data structures in such a way, that the most common access to data is nicely cache
 * optimized. A often dicussed approach to this is to use struct-of-arrays instead of 
 * array-of-structs (source...).
 * 
 * This class is inspired by the excellent blog posts.
 * Managing soa-like data is pretty simple, but a bit cumbersome and error prone (allocating 
 * memory, computing offsets, moving data when inserting or erasing elements), especially 
 * when it comes to changing the data structure (add new arrays/components, changing the type
 * of a component).
 * 
 * I wondered if modern C++11 with its variadic templates and template meta programming [1] 
 * can be of any use here and started to implemented a generic struct-of-arrays container. 
 * Goal is to combine the cache friendly soa approach with the ease of use of 
 * std::vector and similar container classes.
 * 
 * You can think of it as a std::tuple of std::vectors where all vectors are 
 * equally sized.
 * 
 * Goals:
 *  - Use a separate array for every component, so elements of the same type are stored 
 *    sequentially in memory
 *  - Use one continous block of memory for all arrays (just one allocation)
 *  - Use memcpy/memmove for trivial/pod types
 *  - Use constructors/destructors (and move semantics if possible and appropriate) for 
 *    non-trivial types 
 *  - Allow easy iteration over arrays via c++11 range-based for loop
 *  - minimal depedencies 
 *  - support for polymorphic allocators 
 *  - support to specify a different memory alignment for each array
 * 
 * Issues:
 *  - Naming and code convention (right now, its a mixture of std und qt style)
 *  - Copy/move constructors
 *  - Assign/move operators
 *  - moving non-copyable data (like unique_ptr) out of the container
 *    possible solution: move data elements into a std::tuple, but that would add
 *                       a dependency to the tuple header ;(
 *                       I would like to avoid that for such a rarely(?) used feature
 *  - exception safety (especially exception thrown in constructors)
 *  
 * [1] *shudder* yep, its ugly as hell, but it gets the job done... and hopefully you
 * don't have to look at the inner details to use the container...
 */
#pragma once
#include <johl/ArrayRef.h>
#include <johl/internal/Arrays_internal.h>
#include <johl/Allocator.h>
#include <cassert>
#include <cstring>

namespace johl
{
  template<typename... TArrays>
  class Arrays final  
  {
    using ForEach = detail::arrays::ForEach<sizeof...(TArrays), 0, TArrays...>;

    template<size_t Index>
    using Type = typename detail::Get<Index, TArrays...>::Type;

  public: 
    explicit Arrays(Allocator* allocator = Allocator::defaultAllocator())
      : m_numUsed(0)
      , m_numAllocated(0)
      , m_allocator(allocator)
      , m_data(nullptr)
    {
      assert(m_allocator && "allocator must not be null");
      memset(&m_arrays[0], 0, sizeof(m_arrays));
    }

    Arrays(const Arrays&) = delete;
    Arrays& operator=(const Arrays&) = delete;

    ~Arrays()
    {
      clear();
      m_allocator->deallocate(m_data);
    }

    size_t size() const
    {
      return m_numUsed;
    }

    size_t capacity() const
    {
      return m_numAllocated;
    }

    void clear()
    {
      ForEach::destructRange(m_arrays, 0, m_numUsed);
      m_numUsed = 0;
    }

    void reserve(size_t n)
    {
      if(m_numAllocated >= n)
        return;

      void* data = m_allocator->allocate(detail::Size<TArrays...>::value * n);
      void* arrays[sizeof...(TArrays)];

      ForEach::initArrayPointer(arrays, data, n);
      ForEach::moveRange(m_arrays, 0, arrays, 0, m_numUsed);

      m_allocator->deallocate(m_data);

      m_data = data;
      memcpy(&m_arrays[0], &arrays[0], sizeof(m_arrays));

      m_numAllocated = n;
    }

    template<size_t Index>
    ArrayRef<Type<Index>> array()
    {
      return ArrayRef<Type<Index>>(data<Index>(), m_numUsed);
    }

    template<size_t Index>
    ArrayRef<const Type<Index>> array() const
    {
      return ArrayRef<const Type<Index>>(data<Index>(), m_numUsed);
    }

    template<size_t Index>
    Type<Index>* data()
    {
      return static_cast<Type<Index>*>(m_arrays[Index]);
    }

    template<size_t Index>
    const Type<Index>* data() const
    {
      return static_cast<Type<Index>*>(m_arrays[Index]);
    } 

    template<size_t Index>
    auto at(size_t i) -> typename detail::Get<Index, TArrays...>::Type&
    {
      assert(i < m_numUsed && "index i out of range");
      return data<Index>()[i];
    }

    template<size_t Index>
    auto at(size_t i) const -> const typename detail::Get<Index, TArrays...>::Type&
    {
      assert(i < m_numUsed && "index i out of range");
      return data<Index>()[i];
    }

    template<typename... TArgs>
    void append(TArgs... args)
    {
      static_assert(sizeof...(TArgs) == sizeof...(TArrays), "number of arguments does not match number of arrays");

      reserve(m_numUsed + 1);

      ForEach::constructAt(m_arrays, m_numUsed, std::forward<TArgs>(args)...);

      ++m_numUsed;
    }

    void removeAt(size_t index)
    {
      assert(index < m_numUsed && "index out of range");

      ForEach::destructRange(m_arrays, index, 1);      
      ForEach::moveRange(m_arrays, index+1, m_arrays, index, m_numUsed - index - 1);
      --m_numUsed;
    }

    template<typename... TArgs>
    void insertAt(size_t index, TArgs... args)
    {
      static_assert(sizeof...(TArgs) == sizeof...(TArrays), 
        "number of arguments does not match number of arrays");

      assert(index < m_numUsed && "index out of range");

      // reserve space for one extra element, move all elements after index one
      // slot up, construct new element at free slot
      reserve(m_numUsed + 1);
      ForEach::moveRange(m_arrays, index, m_arrays, index+1, m_numUsed - index);
      ForEach::constructAt(m_arrays, index, std::forward<TArgs>(args)...);

      ++m_numUsed;
    }

    void swapAt(size_t a,  size_t b)
    {
      assert(a < m_numUsed && "index a out of range");
      assert(b < m_numUsed && "index b out of range");

      if(a != b)
        ForEach::swap(m_arrays, a, b);      
    }

  private:
    size_t m_numUsed;
    size_t m_numAllocated;
    Allocator* m_allocator;
    void*  m_data;  
    void*  m_arrays[sizeof...(TArrays)];
  };
}