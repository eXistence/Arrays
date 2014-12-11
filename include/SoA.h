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
 * Goals:
 *  - Use a separate array for every component, so elements of the same type are stored 
 *    sequentially in memory
 *  - Use one continous block of memory for all arrays (just one allocation)
 *  - Use memcpy/memmove for trivial/pod types
 *  - Use constructors/destructors (and move semantics if possible and appropriate) for 
 *    non-trivial types 
 *  - Allow easy iteration over arrays via c++11 range-based for loop
 * 
 * Issues:
 *  - Naming and code convention (right now, its a mixture of std und qt style)
 *  - Copy/move constructors
 *  - Assign/move operators
 *  - moving non-copyable data (like unique_ptr) out of the container
 *    possible solution: move data elements into a std::tuple, but that would add
 *                       a dependency to the tuple header ;(
 *                       I would like to avoid that for such a rarely(?) used feature
 *  - Allocator support
 *  - exception safety (especially exception thrown in constructors)
 *  
 * [1] *shudder* yep, its ugly as hell, but it gets the job done... and hopefully you
 * don't have to look at the inner details to use the container...
 */
#pragma once
#include "ArrayRef.h"
#include "detail\SoA_detail.h"
#include <cassert>

namespace johl
{
  template<typename... Args>
  class SoA final
  {
    using ForEach = detail::ForEach<sizeof...(Args), 0, Args...>;

    template<size_t Index>
    using Type = typename detail::Get<Index, Args...>::Type;

  public:
    SoA()
      : m_numUsed(0)
      , m_numAllocated(0)
      , m_data(nullptr)
    {
      memset(&m_arrays[0], 0, sizeof(m_arrays));
    }

    SoA(const SoA&) = delete;
    SoA& operator=(const SoA&) = delete;

    ~SoA()
    {
      clear();
      free(m_data);
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

      void* data = malloc(detail::Size<Args...>::value * n);
      void* arrays[sizeof...(Args)];

      ForEach::initArray(arrays, data, n);
      ForEach::move(m_arrays, arrays, 0, 0, m_numUsed);

      free(m_data);

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
    auto at(size_t i) -> typename detail::Get<Index, Args...>::Type&
    {
      assert(i < m_numUsed && "index i out of range");
      return data<Index>()[i];
    }

    template<size_t Index>
    auto at(size_t i) const -> const typename detail::Get<Index, Args...>::Type&
    {
      assert(i < m_numUsed && "index i out of range");
      return data<Index>()[i];
    }

    template<typename... Args2>
    void append(Args2... args)
    {
      static_assert(sizeof...(Args2) == sizeof...(Args), "number of arguments does not match number of arrays");

      reserve(m_numUsed + 1);

      ForEach::constructAt(m_arrays, m_numUsed, std::forward<Args2>(args)...);

      ++m_numUsed;
    }

    void removeAt(size_t index)
    {
      assert(index < m_numUsed && "index out of range");

      ForEach::destructRange(m_arrays, index, 1);
      --m_numUsed;
      ForEach::move(m_arrays, m_arrays, index+1, index, m_numUsed - index);
    }

    template<typename... Args2>
    void insertAt(size_t index, Args2... args)
    {
      static_assert(sizeof...(Args2) == sizeof...(Args), 
        "number of arguments does not match number of arrays");

      assert(index < m_numUsed && "index out of range");

      reserve(m_numUsed + 1);

      ForEach::move(m_arrays, m_arrays, index, index+1, m_numUsed - index);
      ForEach::constructAt(m_arrays, index, std::forward<Args2>(args)...);

      ++m_numUsed;
    }

    void swapAt(size_t a, size_t b)
    {
      assert(a < m_numUsed && "index a out of range");
      assert(b < m_numUsed && "index b out of range");

      if(a != b)
        ForEach::swap(m_arrays, a, b);      
    }

    void swap(SoA<Args...>& rhs)
    {
      if(this != &rhs)
      {
        //this -> temporary
        const auto tmp_numUsed = m_numUsed;
        const auto tmp_numAllocated = m_numAllocated;
        const auto tmp_data = m_data;
        void* tmp_arrays[sizeof...(Args)];
        memcpy(&tmp_arrays[0], &m_arrays[0], sizeof(tmp_arrays));

        //rhs -> this
        m_numUsed = rhs.m_numUsed;
        m_numAllocated = rhs.m_numAllocated;
        m_data = rhs.m_data;
        memcpy(&m_arrays[0], &rhs.m_arrays[0], sizeof(tmp_arrays));

        //temporary -> rhs
        rhs.m_numUsed = tmp_numUsed;
        rhs.m_numAllocated = tmp_numAllocated;
        rhs.m_data = tmp_data;
        memcpy(&rhs.m_arrays[0], &tmp_arrays[0], sizeof(tmp_arrays));
      }
    }

  private:
    size_t m_numUsed;
    size_t m_numAllocated;
    void*  m_data;  
    void*  m_arrays[sizeof...(Args)];
  };
}