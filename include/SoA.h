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
 * I wondered if modern C++11 can be of any use here and started to implemented a generic 
 * struct-of-arrays container. Goal is to combine the cache friendly soa approach with the ease
 * of use of std::vector and similar container classes.
 * 
 * Goals:
 *  - Use a separate array for every component, so elements of the same type are stored 
 *    sequentially in memory
 *  - Use one continous block of memory for all arrays (just one allocation)
 *  - Use memcpy/memmove for trivial/pod types
 *  - Use constructors/destructors (and move semantics if possible and appropriate) for 
 *    non-trivial types 
 *  - Allow easy iteration over arrays via c++11 'for' loop
 *  
 * [1] *shuffer* yep, its ugly as hell, but it gets the job done... and hopefully you
 * dont have to look at the inner details very often...
 */
#pragma once
#include "detail\SoA_detail.h"

namespace johl
{
  template<typename... Args>
  class SoA
  {
  public:
    SoA()
      : m_numUsed(0)
      , m_numAllocated(0)
      , m_data(nullptr)
    {
      memset(&m_arrays[0], 0, sizeof(m_arrays));
    }

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
      detail::Clear<sizeof...(Args), 0, Args...>::clear(m_arrays, 0, m_numUsed);
      m_numUsed = 0;
    }

    void reserve(size_t n)
    {
      if(m_numAllocated >= n)
        return;

      void* data = malloc(detail::Size<Args...>::value * n);
      void* arrays[sizeof...(Args)];

      detail::SetPointerArray<sizeof...(Args), 0, Args...>::set(arrays, data, n);

      detail::MoveArrays<sizeof...(Args), 0, Args...>::copy(m_arrays, arrays, m_numUsed);

      free(m_data);

      m_data = data;
      memcpy(&m_arrays[0], &arrays[0], sizeof(m_arrays));

      m_numAllocated = n;
    }

    template<size_t Index>
    auto ptr() -> typename detail::Get<Index, Args...>::Type*
    {
      return static_cast<typename detail::Get<Index, Args...>::Type*>(m_arrays[Index]);
    }

    template<typename... Args2>
    void append(Args2... args)
    {
      static_assert(sizeof...(Args2) == sizeof...(Args), "number of arguments does not match number of arrays");

      reserve(m_numUsed + 1);

      detail::Append<sizeof...(Args), 0, Args...>::append(m_arrays, m_numUsed, std::forward<Args2>(args)...);
      ++m_numUsed;
    }


    void removeAt(size_t index)
    {
      detail::Clear<sizeof...(Args), 0, Args...>::clear(m_arrays, index, 1);
      --m_numUsed;
      MoveArrays::copy(m_arrays, m_arrays, index+1, index, m_numUsed - index);
    }

    template<typename... Args2>
    void insertAt(size_t index, Args2... args)
    {
      static_assert(sizeof...(Args2) == sizeof...(Args), "number of arguments does not match number of arrays");

      reserve(m_numUsed + 1);

      MoveArrays::copy(m_arrays, m_arrays, index, index+1, m_numUsed - index);

      detail::Append<sizeof...(Args), 0, Args...>::append(m_arrays, index, std::forward<Args2>(args)...);

      ++m_numUsed;
    }

  /*
    template<size_t Index>
    auto get(size_t i)
    {
      return ptr<Index>()[i];
    }
  */
  private:
    typedef detail::MoveArrays<sizeof...(Args), 0, Args...> MoveArrays;


    size_t m_numUsed;
    size_t m_numAllocated;
    void*  m_data;  
    void*  m_arrays[sizeof...(Args)];
  };
}