#pragma once

#include <type_traits>

namespace johl
{
namespace detail
{
  template<size_t Index, typename... TArrays>
  struct Get;

  template<typename First, typename... Rest>
  struct Get<0, First, Rest...>
  {
    typedef First Type;
  };

  template<size_t Index, typename First, typename... Rest>
  struct Get<Index, First, Rest...>
  {
    static_assert(Index < sizeof...(Rest) + 1, "type index template parameter out of bounds");
    typedef typename Get<Index - 1, Rest...>::Type Type;
  };

  template<typename... TArrays>
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

namespace soa
{

#if 0

  template<size_t RemainingTypes, size_t TypeIndex, typename... TArrays>
  struct CopyArrays;

  template<size_t TypeIndex>
  struct CopyArrays<0, TypeIndex>
  {
    static void copy(void** src_arrays, void** dst_arrays, size_t num)
    {
    }
  };

  template<size_t RemainingTypes, size_t TypeIndex, typename First, typename... Rest>
  struct CopyArrays<RemainingTypes, TypeIndex, First, Rest...>
  {
    static void copy(void** src_arrays, void** dst_arrays, size_t num)
    {
      First* src = static_cast<First*>(src_arrays[TypeIndex]);
      First* dst = static_cast<First*>(dst_arrays[TypeIndex]);

      memcpy(dst, src, sizeof(First) * num);

      CopyArrays<RemainingTypes - 1, TypeIndex + 1, Rest...>::copy(src_arrays, dst_arrays, num);
    }
  };

#endif
  
  template<class T>
  typename std::enable_if<std::is_trivially_destructible<T>::value && std::is_trivially_copyable<T>::value, void>::type 
    moveData(T* dst, const T* src, size_t num)
  {
      memmove(dst, src, sizeof(T) * num);   
  }

  template<class T>
  typename std::enable_if<!(std::is_trivially_destructible<T>::value && std::is_trivially_copyable<T>::value), void>::type
    moveData(T* dst, T* src, size_t num)
  {
      auto f = [=](size_t index)
      {
        new (&dst[index]) T(std::move(src[index]));
        src[index].~T();
      };

      if (dst < src)
      {
        for (size_t i = 0; i < num; ++i)
          f(i);
      }
      else
      {
        for (size_t i = num; i > 0; --i)
          f(i - 1);
      }
  }
  
  template<class T>
  typename std::enable_if<std::is_trivially_destructible<T>::value, void>::type
    destructArrayElements(T* array, size_t num)
  {
  }

  template<class T>
  typename std::enable_if<!std::is_trivially_destructible<T>::value, void>::type
    destructArrayElements(T* array, size_t num)
  {
      for (size_t i = 0; i < num; ++i)
      {
        array[i].~T();
      }
  }
  
  template<size_t RemainingTypes, size_t TypeIndex, typename... TArrays>
  struct ForEach;

  //this specialized template (with RemainingTypes == 0) marks the end of
  // the meta program recursion
  template<size_t TypeIndex>
  struct ForEach<0, TypeIndex>
  {
    static void initArrayPointer(void** arrays, void* data, size_t numAllocated) {}
    static void destructRange(void** arrays, size_t from, size_t num) {}
    static void constructAt(void** arrays, size_t index) {}
    static void moveRange(void** src_arrays, size_t src_from, void** dst_arrays, size_t dst_from, size_t num)  {}
    static void swap(void** arrays, size_t a, size_t b) {}
  };

  template<size_t RemainingTypes, size_t TypeIndex, typename First, typename... Rest>
  struct ForEach<RemainingTypes, TypeIndex, First, Rest...>
  {
    using Next = ForEach<RemainingTypes-1, TypeIndex+1, Rest...>;

    static void initArrayPointer(void** arrays, void* data, size_t numAllocated)
    {
      char** arr = (char**)arrays;
      char*  d = (char*)data;

      arr[TypeIndex] = d;

      Next::initArrayPointer(arrays, &d[sizeof(First) * numAllocated], numAllocated);
    }

    static void destructRange(void** arrays, size_t from, size_t num)
    {
      First* array = static_cast<First*>(arrays[TypeIndex]);
      destructArrayElements(&array[from], num);

      Next::destructRange(arrays, from, num);
    }

    template<typename FirstArg, typename... RestArgs>
    static void constructAt(void** arrays, size_t index, FirstArg first, RestArgs ...rest)
    {
      First* data = static_cast<First*>(arrays[TypeIndex]);

      new (&data[index]) First(std::forward<FirstArg>(first));

      Next::constructAt(arrays, index, std::forward<RestArgs>(rest)...);
    }

    static void moveRange(void** src_arrays, size_t src_from, void** dst_arrays, size_t dst_from, size_t num)
    {
      First* src = static_cast<First*>(src_arrays[TypeIndex]);
      First* dst = static_cast<First*>(dst_arrays[TypeIndex]);

      moveData(&dst[dst_from], &src[src_from], num);

      Next::moveRange(src_arrays, src_from, dst_arrays, dst_from, num);
    }

    static void swap(void** arrays, size_t a, size_t b)
    {
      First* array = static_cast<First*>(arrays[TypeIndex]);

      First& oa = array[a];
      First& ob = array[b];

      First tmp = std::move(oa);
      oa = std::move(ob);
      ob = std::move(tmp);

      Next::swap(arrays, a, b);
    }
  };
}
}
}