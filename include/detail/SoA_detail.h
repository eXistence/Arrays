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



  template<size_t RemainingTypes, size_t TypeIndex, typename... Args>
  struct SetPointerArray;

  template<size_t TypeIndex, typename... Args>
  struct SetPointerArray<0, TypeIndex, Args...>
  {
    static void set(void** arrays, void* data, size_t num)
    {
    }
  };

  template<size_t RemainingTypes, size_t TypeIndex, typename... Args>
  struct SetPointerArray<RemainingTypes, TypeIndex, Args...>
  {
    static void set(void** arrays, void* data, size_t num)
    {
      char** arr = (char**)arrays;
      char*  d = (char*)data;

      arr[TypeIndex] = &d[ Offset<TypeIndex, Args...>::value * num ];

      SetPointerArray<RemainingTypes-1, TypeIndex+1, Args...>::set(arrays, data, num);
    }

  };





  template<size_t RemainingTypes, size_t TypeIndex, typename... Args>
  struct Append;

  template<size_t TypeIndex>
  struct Append<0, TypeIndex>
  {
    static void append(void** /*arrays*/, size_t /*index*/)
    {
    }
  };

  template<size_t RemainingTypes, size_t TypeIndex, typename First, typename... Rest>
  struct Append<RemainingTypes, TypeIndex, First, Rest...>
  {
    template<typename FirstArg, typename... RestArgs>
    static void append(void** arrays, size_t index, FirstArg first, RestArgs ...rest)
    {
      First* data = static_cast<First*>(arrays[TypeIndex]);

      //data[index] = first;
      new (&data[index]) First(first);

      Append<RemainingTypes-1, TypeIndex+1, Rest...>::append(arrays, index, std::forward<RestArgs>(rest)...);
    }
  }; 


  template<size_t RemainingTypes, size_t TypeIndex, typename... Args>
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







  template<class T>
  typename std::enable_if<std::is_trivially_destructible<T>::value && std::is_trivially_copyable<T>::value, void>::type 
    moveData(T* dst, const T* src, size_t num)
  {
      memmove(dst, src, sizeof(T) * num);   
  }

  template<class T>
  typename std::enable_if<!(std::is_trivially_destructible<T>::value && std::is_trivially_copyable<T>::value), void>::type
    moveData(T* dst, const T* src, size_t num)
  {
      if(dst < src)
      {
        for (size_t i = 0; i < num; ++i)
        {
          new (&dst[i]) T(std::move(src[i]));
          src[i].~T();
        }
      }
      else
      {
        for (size_t i = num; i > 0; --i)
        {
          size_t index = i-1;

          new (&dst[index]) T(std::move(src[index]));
          src[index].~T();
        }
      }
  }

  

  template<size_t RemainingTypes, size_t TypeIndex, typename... Args>
  struct MoveArrays;

  template<size_t TypeIndex>
  struct MoveArrays<0, TypeIndex>
  {
    static void copy(void** src_arrays, void** dst_arrays, size_t num)
    {
    }

    static void copy(void** src_arrays, void** dst_arrays, size_t src_offset, size_t dst_offset, size_t num)
    {
    }
  };

  template<size_t RemainingTypes, size_t TypeIndex, typename First, typename... Rest>
  struct MoveArrays<RemainingTypes, TypeIndex, First, Rest...>
  {
    static void copy(void** src_arrays, void** dst_arrays, size_t num)
    {
      copy(src_arrays, dst_arrays, 0, 0, num);
    }

    static void copy(void** src_arrays, void** dst_arrays, size_t src_offset, size_t dst_offset, size_t num)
    {
      First* src = static_cast<First*>(src_arrays[TypeIndex]);
      First* dst = static_cast<First*>(dst_arrays[TypeIndex]);

      moveData(&dst[dst_offset], &src[src_offset], num);
      //      memcpy(dst, src, sizeof(First)* num);

      MoveArrays<RemainingTypes - 1, TypeIndex + 1, Rest...>::copy(src_arrays, dst_arrays, src_offset, dst_offset, num);
    }
  };










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

  template<size_t RemainingTypes, size_t TypeIndex, typename... Args>
  struct Clear;

  template<size_t TypeIndex>
  struct Clear<0, TypeIndex>
  {
    static void clear(void** arrays, size_t indexOffset, size_t num)
    {
    }
  };

  template<size_t RemainingTypes, size_t TypeIndex, typename First, typename... Rest>
  struct Clear<RemainingTypes, TypeIndex, First, Rest...>
  {
    static void clear(void** arrays, size_t indexOffset, size_t num)
    {
      First* array = static_cast<First*>(arrays[TypeIndex]);
      destructArrayElements(&array[indexOffset], num);

      Clear<RemainingTypes-1, TypeIndex+1, Rest...>::clear(arrays, indexOffset, num);
    }
  };

}
}
