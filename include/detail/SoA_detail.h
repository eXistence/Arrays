#pragma once

#include <type_traits>

namespace johl
{
namespace detail
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
    typedef typename Get<Index - 1, Rest...>::Type Type;
  };






  template<typename... Args>
  struct Size;

  template<typename First, typename... Rest>
  struct Size<First, Rest...>
  {
    static const size_t value = sizeof(First)+Size<Rest...>::value;
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
    static const size_t value = sizeof(First)+Offset<Index - 1, Rest...>::value;
  };







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
      for (size_t i = 0; i < num; ++i)
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
      for (size_t i = 0; i < num; ++i)
      {
        new (dst[i]) T(std::move(from[i]));
      }
  }



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
}
