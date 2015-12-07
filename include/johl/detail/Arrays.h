#pragma once

#include <type_traits>
#include <utility>
#include <cstring>
#include <cstdint>

#include <ciso646>

namespace johl
{
namespace detail
{  
//we assume type traits to be available if
// - standard library is 'libc++' (instead of libstdc++)  or
// - compiler is msvc (type traits are implemented since VS2013/compiler version 18.x))  or
// - compiler is gcc 5.x (technically its still possible that an older version of libstdc++ is used, 
//   that does not support type traits yet, but thats unlikely)
#if defined(_LIBCPP_VERSION) || (_MSC_VER >= 1800) || (__GNUC__ >= 5)
  template<typename T>
  using is_trivially_copyable = std::is_trivially_copyable<T>;

  template<typename T>
  using is_trivially_destructible = std::is_trivially_destructible<T>;
#elif(__GNUC__)
//no type traits available, define our own 'poor mans' type traits based on gcc language extensions
  template<typename T>
  struct is_trivially_copyable
  {
    static const bool value = __has_trivial_copy(T);
  };

  template<typename T>
  struct is_trivially_destructible
  {
    static const bool value = __has_trivial_destructor(T);
  };
#endif  

  /**
   * Helper function to silence compiler warnings for unused parameters.
   * Every halfway decent optimizer will remove calls to this function completely.
   */
  template<typename... T>
  void unused(const T&...)
  {
  }

  /**
   * Template meta program to get the n-th type from a parameter pack (TArrays).
   */
  template<size_t Index, typename... TArrays>
  struct Get;

  template<typename First, typename... Rest>
  struct Get<0, First, Rest...> final
  {
    Get() = delete;
    typedef First Type; //end of template meta program recursion
  };

  template<size_t Index, typename First, typename... Rest>
  struct Get<Index, First, Rest...> final
  {
    Get() = delete;
    static_assert(Index < sizeof...(Rest) + 1, "type index template parameter out of bounds");
    typedef typename Get<Index - 1, Rest...>::Type Type;
  };
  
  /**
   * check at compile time, if N is a power of two
   */
  template<int N>
  struct is_power_of_two final
  {
    is_power_of_two() = delete;

    static const int value = N && !(N & (N - 1));
  };
  
  /**
   * AlignedType::Type is always the actual type.
   * AlignedType::align is the default alignment.
   * this type will be specialized for the 'align' tag type, that override the
   * alignment.
   */
  template<typename T>
  struct AlignedType final
  {
    AlignedType() = delete;
    using Type = T;
    static const size_t align = 4;
  }; 

  /**
   * template meta program to calculate the sum of all alignments for a given
   * list of types.
   */
  template<typename... Types>
  struct SumAlignment;

  template<typename T>
  struct SumAlignment<T> final
  {
    SumAlignment() = delete;
    static const size_t value = AlignedType<T>::align;
  };

  template<typename TFirst, typename... TRest>
  struct SumAlignment<TFirst, TRest...> final
  {
    SumAlignment() = delete;
    static const size_t value = AlignedType<TFirst>::align + SumAlignment<TRest...>::value;
  };


  /**
   * template meta program to calculate the sum of all sizes for a given
   * list of types.
   */
  template<typename... Types>
  struct SumSize;

  template<typename T>
  struct SumSize<T>
  {
    static const size_t value = sizeof(typename AlignedType<T>::Type);
  };

  template<typename TFirst, typename... TRest>
  struct SumSize<TFirst, TRest...>
  {
    static const size_t value = sizeof(typename AlignedType<TFirst>::Type) + SumSize<TRest...>::value;
  };

namespace arrays
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

  /**
   * move trivial data from src to dst by calling memmove.
   * 
   * This function is removed from overload resolution if type T is not trivially destructible or not trivially copyable.
   */
  template<class T>
  typename std::enable_if<is_trivially_destructible<T>::value && is_trivially_copyable<T>::value, void>::type 
    moveData(T* dst, const T* src, size_t num)
  {
      memmove(dst, src, sizeof(T) * num);   
  }

  /**
   * move non-trivial data from src to dst.
   * 'move' means in this case:
   *   - move-construct object at dst[i] (assumes dst is raw memory! moves from src[i])
   *   - destruct old (moved-from) object at src[i]
   *   - does not allocate any memory, does not free any memory
   *   
   * This function is removed from overload resolution if type T is trivially destructible and trivially copyable.
   */
  template<class T>
  typename std::enable_if<!(is_trivially_destructible<T>::value && is_trivially_copyable<T>::value), void>::type
    moveData(T* dst, T* src, size_t num)
  {
      auto f = [=](size_t index)
      {
        new (&dst[index]) T(std::move(src[index])); //move-construct at dst
        src[index].~T();                            //destruct old object
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
  
  /**
   * Call destructor for a given range of objects.    
   * Enabled only for trivially destructible types (does nothing).
   */
  template<class T>
  typename std::enable_if<is_trivially_destructible<T>::value, void>::type
    destructArrayElements(T* array, size_t num)
  {
    unused(array, num);
  }

  /**
   * Call destructor for a given range of objects. 
   * Disabled for trivially destructible types.
   */
  template<class T>
  typename std::enable_if<!is_trivially_destructible<T>::value, void>::type
    destructArrayElements(T* array, size_t num)
  {
      for (size_t i = 0; i < num; ++i)
      {
        array[i].~T();
      }
  }
  
  /**
   * 
   */
  template<size_t RemainingTypes, size_t TypeIndex, typename... TArrays>
  struct ForEach;

  //this specialized template (with RemainingTypes == 0) marks the end of
  // the template meta program recursion
  template<size_t TypeIndex>
  struct ForEach<0, TypeIndex>
  {
    static void initArrayPointer(void** arrays, void* data, size_t numAllocated)
    { 
      unused(arrays, data, numAllocated); 
    }

    static void destructRange(void** arrays, size_t from, size_t num) 
    { 
      unused(arrays, from, num); 
    }

    static void constructAt(void** arrays, size_t index)
    { 
      unused(arrays, index);
    }

    static void moveRange(void** src_arrays, size_t src_from, void** dst_arrays, size_t dst_from, size_t num)
    {
      unused(src_arrays, src_from, dst_arrays, dst_from, num);
    }

    static void swap(void** arrays, size_t a, size_t b)
    {
      unused(arrays, a, b);
    }
  };
  
  template<size_t RemainingTypes, size_t TypeIndex, typename First, typename... Rest>
  struct ForEach<RemainingTypes, TypeIndex, First, Rest...>
  {
    using Next = ForEach<RemainingTypes-1, TypeIndex+1, Rest...>;

    using CurrentType = typename AlignedType<First>::Type;
    static const size_t currentAlignment = AlignedType<First>::align;
    static_assert(is_power_of_two<currentAlignment>::value, "alignement needs to be power of two");

    static void initArrayPointer(void** arrays, void* data, size_t numAllocated)
    {
      char** arr = (char**)arrays;

      auto p = (std::uintptr_t)data;
      p = p + (currentAlignment - (p % currentAlignment));
      char*  d = (char*)p;

      arr[TypeIndex] = d;

      Next::initArrayPointer(arrays, &d[sizeof(CurrentType) * numAllocated], numAllocated);
    }

    static void destructRange(void** arrays, size_t from, size_t num)
    {
      CurrentType* array = static_cast<CurrentType*>(arrays[TypeIndex]);
      destructArrayElements(&array[from], num);

      Next::destructRange(arrays, from, num);
    }

    template<typename FirstArg, typename... RestArgs>
    static void constructAt(void** arrays, size_t index, FirstArg first, RestArgs ...rest)
    {
      CurrentType* data = static_cast<CurrentType*>(arrays[TypeIndex]);

      new (&data[index]) CurrentType(std::forward<FirstArg>(first));

      Next::constructAt(arrays, index, std::forward<RestArgs>(rest)...);
    }

    static void moveRange(void** src_arrays, size_t src_from, void** dst_arrays, size_t dst_from, size_t num)
    {
      CurrentType* src = static_cast<CurrentType*>(src_arrays[TypeIndex]);
      CurrentType* dst = static_cast<CurrentType*>(dst_arrays[TypeIndex]);

      moveData(&dst[dst_from], &src[src_from], num);

      Next::moveRange(src_arrays, src_from, dst_arrays, dst_from, num);
    }

    static void swap(void** arrays, size_t a, size_t b)
    {
      CurrentType* array = static_cast<CurrentType*>(arrays[TypeIndex]);

      CurrentType& oa = array[a];
      CurrentType& ob = array[b];

      CurrentType tmp = std::move(oa);
      oa = std::move(ob);
      ob = std::move(tmp);

      Next::swap(arrays, a, b);
    }
  };
}
}
}