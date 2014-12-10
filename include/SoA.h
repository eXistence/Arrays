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
      if(m_data)
        free(m_data);
    }

    void reserve(size_t n)
    {
      if(m_numAllocated >= n)
        return;

      const size_t bytes = detail::Size<Args...>::value * n;

      void* data = malloc(bytes);

      if(m_data)
        free(m_data);

      m_data = data;

      detail::Foo<Args...>::initPointers((char**)&m_arrays[0], (char*)m_data, n);
    }

    template<size_t Index>
    auto ptr() -> typename detail::Get<Index, Args...>::Type*
    {
      return static_cast<typename detail::Get<Index, Args...>::Type*>(m_arrays[Index]);
    }
  /*
    void append(Args... args)
    {
      reserve(m_numUsed + 1);
    }

    template<size_t Index>
    auto get(size_t i)
    {
      return ptr<Index>()[i];
    }
  */
  private:
    size_t m_numUsed;
    size_t m_numAllocated;
    void*  m_data;  
    void*  m_arrays[sizeof...(Args)];
  };
}