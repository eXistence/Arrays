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


#include <iostream>
#include <cstdint>
#include <typeinfo>
#include <string>
#include <cstring>
#include <cstddef>

using std::ptrdiff_t;

namespace meta
{
  template<typename... Args>
  struct TypeList;

  template<typename First, typename... Rest>
  struct TypeList<First, Rest...>
  {
    typedef First Type;
    typedef TypeList<Rest...> List;
  };

  template<typename First>
  struct TypeList<First>
  {    
    typedef First Type;
  };






  template<size_t Index, typename... Args>
  struct Get;

  template<typename First, typename... Rest>
  struct Get<0, First, Rest...>
  {
    typedef First Type;
  };

  template<size_t Index, typename First, typename... Rest>
  struct Get<Index, First, Rest...>
  {
    typedef typename Get<Index-1, Rest...>::Type Type;
  };  






  template<typename... Args>
  struct Size;

  template<typename First, typename... Rest>
  struct Size<First, Rest...>
  {
    static const size_t value = sizeof(First) + Size<Rest...>::value;    
  };

  template<>
  struct Size<>
  {
    static const size_t value = 0;    
  };  




  template<size_t Index, typename... Args>
  struct Offset;

  template<typename First, typename... Rest>
  struct Offset<0, First, Rest...>
  {
    static const size_t value = 0;
  };

  template<size_t Index, typename First, typename... Rest>
  struct Offset<Index, First, Rest...>
  {
    static const size_t value = sizeof(First) + Offset<Index-1, Rest...>::value;
  };  
}

namespace arrays
{
  template<class T>
  typename std::enable_if<std::is_trivially_copy_constructible<T>::value, T>::type 
      move_copy(T* dst, const T* from, size_t num) 
  {
    memcpy(dst, from, num * sizeof(T));
  }
 
  template<class T>
  typename std::enable_if<!std::is_trivially_copy_constructible<T>::value, T>::type 
      move_copy(T* dst, const T* from, size_t num) 
  {
    for(size_t i=0; i<num; ++i)
      dst[i] = std::move(from[i]);
  }

  template<class T>
  typename std::enable_if<std::is_trivially_copy_constructible<T>::value, T>::type 
      move_construct(T* dst, const T* from, size_t num) 
  {
    memcpy(dst, from, num * sizeof(T));
  }
 
  template<class T>
  typename std::enable_if<!std::is_trivially_copy_constructible<T>::value, T>::type 
      move_construct(T* dst, const T* from, size_t num) 
  {
    for(size_t i=0; i<num; ++i)
    {
      new (dst[i]) T(std::move(from[i]));
    }
  }  
}


using namespace meta;

typedef TypeList<int, float, std::string> MyTypes;


namespace soa_internal
{
  template<typename... Args>
  struct Foo;

  template<typename First, typename... Rest>
  struct Foo<First, Rest...>
  {
    static void initPointers(char** pointers, char* data, size_t num)
    {
      pointers[0] = data;
      Foo<Rest...>::initPointers(&pointers[1], &data[Offset<1, First, Rest...>::value * num], num);
    }
  };

  template<typename First>
  struct Foo<First>
  {
    static void initPointers(char** pointers, char* data, size_t /*num*/)
    {
      pointers[0] = data;
    }
  };  
}


template<typename T>
class ArrayRef
{
public:

private:
  T*     m_data;
  size_t m_size;
};

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

    const size_t bytes = Size<Args...>::value * n;
    std::cout << "allocating " << bytes << "b of data" << std::endl;

    void* data = malloc(bytes);

    if(m_data)
      free(m_data);

    m_data = data;

    soa_internal::Foo<Args...>::initPointers((char**)&m_arrays[0], (char*)m_data, n);
  }

  void printArrayOffsets()
  {
    if(m_data == nullptr)
      std::cout << "no arrays available" << std::endl;

    for(int i=0; i<sizeof...(Args); ++i)
    {
      std::cout << " " << i << ": " << ((ptrdiff_t)m_arrays[i] - (ptrdiff_t)m_data) << std::endl;
    }
  }

  template<size_t Index>
  auto ptr() -> typename meta::Get<Index, Args...>::Type*
  {
    return static_cast<typename Get<Index, Args...>::Type*>(m_arrays[Index]);
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

