#pragma once
#include <johl/ArrayRef.h>
#include <johl/detail/Arrays.h>
#include <johl/Allocator.h>
#include <cassert>
#include <cstring>

namespace johl
{
  /**
   * Tag type that annotates a type with a given alignment.
   */
  template<typename TType, size_t TAlign>
  struct aligned final
  {
    static_assert(detail::is_power_of_two<TAlign>::value, "TAlign must be power two");
    aligned() = delete;

    using Type = TType;
    static const size_t align = TAlign;
  };

  /**
   * 'struct-of-arrays like' container. Maintains multiple arrays that are all
   * equally sized. 
   */
  template<typename... TArrays>
  class Arrays final  
  {
  private:
    //This type implements the actual template meta program.
    //Contains static helper functions, that the compiler will instantiate and call
    // for each array.
    using ForEachArray = detail::arrays::ForEach<sizeof...(TArrays), 0, TArrays...>;

    //Short hand to get a type from the parameter pack (TArrays)
    template<size_t Index>
    using Type = typename detail::AlignedType<typename detail::Get<Index, TArrays...>::Type>::Type;

  public: 
    explicit Arrays(Allocator* allocator = Allocator::defaultAllocator());

    Arrays(const Arrays&) = delete;
    Arrays& operator=(const Arrays&) = delete;

    ~Arrays();

    size_t size() const;
    size_t capacity() const;
    void clear();
    void reserve(size_t n);

    template<size_t Index>
    ArrayRef<Type<Index>> array();

    template<size_t Index>
    ArrayRef<const Type<Index>> array() const;

    template<size_t Index>
    Type<Index>* data();

    template<size_t Index>
    const Type<Index>* data() const;

    template<size_t Index>
    auto at(size_t i) -> typename detail::Get<Index, TArrays...>::Type&;

    template<size_t Index>
    auto at(size_t i) const -> const typename detail::Get<Index, TArrays...>::Type&;

    template<typename... TArgs>
    void append(TArgs... args);

    void removeAt(size_t index);

    template<typename... TArgs>
    void insertAt(size_t index, TArgs... args);

    void swapAt(size_t a,  size_t b);

  private:
    size_t m_numUsed;
    size_t m_numAllocated;
    Allocator* m_allocator;
    void*  m_data;  
    void*  m_arrays[sizeof...(TArrays)];
  };

  //============================================================================

  namespace detail
  {
    //specialize helper type AlignedType for the 'align' tag type.
    //AlignedType::Type is always the actual type.
    //AlignedType::align is TAlign for align types, otherwise its a constant 
    // default value (see details header)
    template<typename T, size_t TAlign>
    struct AlignedType<aligned<T, TAlign>> final
    {
      AlignedType() = delete;

      using Type = T;
      static const size_t align = TAlign;
    };
  }

  template<typename... TArrays>
  Arrays<TArrays...>::Arrays(Allocator* allocator)
    : m_numUsed(0)
    , m_numAllocated(0)
    , m_allocator(allocator)
    , m_data(nullptr)
  {
    assert(m_allocator && "allocator must not be null");
    memset(&m_arrays[0], 0, sizeof(m_arrays));
  }

  template<typename... TArrays>
  Arrays<TArrays...>::~Arrays()
  {
    clear();
    m_allocator->deallocate(m_data);
  }

  template<typename... TArrays>
  size_t Arrays<TArrays...>::size() const
  {
    return m_numUsed;
  }

  template<typename... TArrays>
  size_t Arrays<TArrays...>::capacity() const
  {
    return m_numAllocated;
  }

  template<typename... TArrays>
  void Arrays<TArrays...>::clear()
  {
    ForEachArray::destructRange(m_arrays, 0, m_numUsed);
    m_numUsed = 0;
  }

  template<typename... TArrays>
  void Arrays<TArrays...>::reserve(size_t n)
  {
    if (m_numAllocated >= n)
      return;

    const size_t bytes = (detail::SumSize<TArrays...>::value * n) + detail::SumAlignment<TArrays...>::value;
    void* data = m_allocator->allocate(bytes);
    void* arrays[sizeof...(TArrays)];

    ForEachArray::initArrayPointer(arrays, data, n);
    ForEachArray::moveRange(m_arrays, 0, arrays, 0, m_numUsed);

    m_allocator->deallocate(m_data);

    m_data = data;
    memcpy(&m_arrays[0], &arrays[0], sizeof(m_arrays));

    m_numAllocated = n;
  }

  template<typename... TArrays>
  template<size_t Index>
  auto Arrays<TArrays...>::array() -> ArrayRef<Type<Index>>
  {
    return ArrayRef<Type<Index>>(data<Index>(), m_numUsed);
  }

  template<typename... TArrays>
  template<size_t Index>
  auto  Arrays<TArrays...>::array() const -> ArrayRef<const Type<Index>>
  {
    return ArrayRef<const Type<Index>>(data<Index>(), m_numUsed);
  }

  template<typename... TArrays>
  template<size_t Index>
  auto Arrays<TArrays...>::data() -> Type<Index>*
  {
    return static_cast<Type<Index>*>(m_arrays[Index]);
  }

  template<typename... TArrays>
  template<size_t Index>
  auto Arrays<TArrays...>::data() const -> const Type<Index>*
  {
    return static_cast<Type<Index>*>(m_arrays[Index]);
  }

  template<typename... TArrays>
  template<size_t Index>
  auto Arrays<TArrays...>::at(size_t i) -> typename detail::Get<Index, TArrays...>::Type&
  {
    assert(i < m_numUsed && "index i out of range");
    return data<Index>()[i];
  }

  template<typename... TArrays>
  template<size_t Index>
  auto Arrays<TArrays...>::at(size_t i) const -> const typename detail::Get<Index, TArrays...>::Type&
  {
    assert(i < m_numUsed && "index i out of range");
    return data<Index>()[i];
  }

  template<typename... TArrays>
  template<typename... TArgs>
  void Arrays<TArrays...>::append(TArgs... args)
  {
    static_assert(sizeof...(TArgs) == sizeof...(TArrays), "number of arguments does not match number of arrays");

    reserve(m_numUsed + 1);

    ForEachArray::constructAt(m_arrays, m_numUsed, std::forward<TArgs>(args)...);

    ++m_numUsed;
  }

  template<typename... TArrays>
  void Arrays<TArrays...>::removeAt(size_t index)
  {
    assert(index < m_numUsed && "index out of range");

    ForEachArray::destructRange(m_arrays, index, 1);
    ForEachArray::moveRange(m_arrays, index + 1, m_arrays, index, m_numUsed - index - 1);
    --m_numUsed;
  }

  template<typename... TArrays>
  template<typename... TArgs>
  void Arrays<TArrays...>::insertAt(size_t index, TArgs... args)
  {
    static_assert(sizeof...(TArgs) == sizeof...(TArrays),
      "number of arguments does not match number of arrays");

    assert(index < m_numUsed && "index out of range");

    // reserve space for one extra element, move all elements after index one
    // slot up, construct new element at free slot
    reserve(m_numUsed + 1);
    ForEachArray::moveRange(m_arrays, index, m_arrays, index + 1, m_numUsed - index);
    ForEachArray::constructAt(m_arrays, index, std::forward<TArgs>(args)...);

    ++m_numUsed;
  }

  template<typename... TArrays>
  void Arrays<TArrays...>::swapAt(size_t a, size_t b)
  {
    assert(a < m_numUsed && "index a out of range");
    assert(b < m_numUsed && "index b out of range");

    if (a != b)
      ForEachArray::swap(m_arrays, a, b);
  }
}
