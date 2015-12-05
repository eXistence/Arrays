#pragma once
#include <johl/ArrayRef.h>
#include <johl/detail/Arrays.h>
#include <johl/Allocator.h>
#include <cassert>
#include <cstring>

namespace johl
{
  template<typename TType, size_t TAlign>
  struct aligned
  {
    static_assert(detail::is_power_of_two<TAlign>::value, "TAlign must be power two");
    using Type = TType;
    static const size_t align = TAlign;
  };

  namespace detail
  {
    template<typename T, size_t TAlign>
    struct AlignedType<aligned<T, TAlign>>
    {
      using Type = T;
      static const size_t align = TAlign;
    };
  }

  template<typename... TArrays>
  class Arrays final  
  {
    using ForEach = detail::arrays::ForEach<sizeof...(TArrays), 0, TArrays...>;

    template<size_t Index>
    using Type = typename detail::AlignedType<typename detail::Get<Index, TArrays...>::Type>::Type;

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

      const size_t bytes = detail::SumSize<TArrays...>::value * n + detail::SumAlignment<TArrays...>::value;
      void* data = m_allocator->allocate(bytes);
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
