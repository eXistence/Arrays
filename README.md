


Introduction
===============

A generic implementation of a struct-of-arrays (soa) container. Inspired by a couple of articles and talks about data oriented design (DOD).

(More details on DOD coming soon)

Managing soa-like data in a language like C is pretty straight-forward and simple. Thats beeing said, it is still a bit cumbersome and is prone to errors, especially if you need to change the structure of your data (adding and removing elements, changing types, tweaking layout, sizes and alignment). I wondered if modern C++11 with its variadic templates and template meta programming can be of any use here. Goal is to combine the cache friendly soa approach with the ease of use and type safety of std::vector and similar container classes.

(More details on soa coming soon)

Features
===============
 - Provides a separate array for each component, so elements of the same type are stored sequentially in memory.
 - Memory managment
   - Uses one continous block of memory for all arrays (just one allocation! less pressure on the allocator, less memory fragmentation).
   - Support for stateful/polymorphic allocators.
   - Ability to specify a different memory alignment for each array. 
 - C++11
   - Actually requires C++11 or later (for type traits, enable_if and variadic templates)
   - Supports iteration over arrays via C++11 range-based for-loop.
 - type safe
 - const correct
 - Support for all kinds of data
   - Uses memcpy/memmove for trivial/pod types.
   - Uses constructors/destructors (and move semantics if possible and appropriate) for non-trivial types. 
   - trivial and non-trivial types can be mixed
 - Easy to integrate
   - Header-only library (CMake is only required to compile the unit tests and benchmarks)
   - No dependencies (no boost! only minimal usage of C++ standard library).   
 - Cross platform
    - gcc
    - clang
    - visual studio 
 - Passes valgrinds memcheck tests
 - Passes clangs address sanatizers

How to use it
===============

* add the `include` directory to your compiler's list of include directories
* simple use cases
  ```cpp
  #include <johl/Arrays.h>
  using namespace johl;

  //Create an Arrays object, trivial and non-trivial types can be freely mixed
  Arrays<float, int, std::string> myarrays;

  //reserve space for 3 rows 
  myarrays.reserve(3);

  //append data
  //all arrays are always equally sized, so you always have to fully initialize 
  //a row (you can not add only the float and the int sometimes after that).
  myarrays.append(1.0f, 100, "one");
  myarrays.append(2.0f, 200, "two");
  myarrays.append(3.0f, 300, "three");

  //get arrays as pointers and use them to iterate over your data 
  //or to pass them to some other api
  float* arr_f = myarrays.data<0>();
  int* arr_i = myarrays.data<1>();

  for(size_t i=0; i<myarrays.size(); ++i) {
    arr_f[i] += arr_i[i];
  }

  //use C++11 range-based for-loop to iterate over an array
  for(const auto& s : myarrays.array<2>()) {
    std::cout << s << std::endl;
  }
  ```
* custom allocator 
  ```cpp
  #include <johl/Arrays.h>
  using namespace johl;

  class MyAllocator : public Allocator {
    virtual void* allocate(size_t size) override { ... };
    virtual void deallocate(void* p) override { ... };
  };

  //create allocator, the allocator must outlive the Arrays object!
  MyAllocator myalloc;

  //create Arrays object
  Arrays<float, int> myarrays(&myalloc);
  
  //use myarrays just like before  
  ```

* memory alignment 
  ```cpp
  #include <johl/Arrays.h>
  using namespace johl;

  //create an Arrays object.
  //use the 'aligned' tag type to specify the alignment of an array 
  // in this case, the float array is aligned to 16 bytes.
  Arrays<aligned<float, 16>, int> myarrays;
  
  float* f = myarrays.data<0>();
  assert((uintptr_t)f % 16 == 0);

  //use myarrays just like before    
  ```  


Benchmarks
===============

coming soon

Personal Notes
===============

 * ToDo's:
   * Code not fully documented yet (read: not documented at all ;))
   * The container does not use exceptions by itself (so you can use it even if you disabled exception support in the language), but i still need to check if all code paths are ok with exceptions thrown from the users code (e.g. from a constructor of an element type).
   * If the container needs to grow, it increases itself by the size of only one row. This can lead to a lot of allocations/deallocations. std::vector's growth rate is implementation defined but is often something like 2^n. I think that's a bit extreme and leads to a lot of over-growth. Maybe i should always grow by blocks of 16 rows or something like that. For the time beeing its highly recommended to call `reserve` to allocate all the memory you need upfront. That's what you usually should do anyways, even with std::vector.
   * Arrays is not a value type (yet?). Copy construction and assignments are deleted. Not sure yet how to deal with allocators in those cases. 
   * Move construction not implemented yet.
   * Comparison operators not implemented yet
   * Some edge cases for move-only element types (like std::unique_ptr) are a bit inconvenient (but move-only types should work in general)
   * test more compiler versions
     * visual studio tested only with version 2013, vs2015 should be fine too, not sure about vs2012 (would definitely need macro _VARIADIC_MAX=10... need to test that)
     * gcc tested only with version 4.9 and 5.2 (i expect every version >= 4.7 to be ok, but have not tested that yet)
     * clang tested only with version 3.6
 * When doing micro benchmarks, always initialize your data. All of it! Even if you don't need the data or don't care about the actual values. Otherwise the optimizer can do crazy things to your code...
 * Dont trust valgrind's cachegrind results. Don't get me wrong on this, cachegrind is a great tool to profile your code and to find bottlenecks. The only problems with cachegrind is that cachegrind only simulates branch prediction and caches. Real hardware behaves often slightly differently and can produce different results, if those details are important to you, use something like `perf` to test on the real thing.

References
===============
 * Series of blogposts about the entity/component system build into the bitsquid engine
   * [Building a Data-Oriented Entity System (Part 1)](http://bitsquid.blogspot.de/2014/08/building-data-oriented-entity-system.html)
   * [Building a Data-Oriented Entity System (Part 2: Components)](http://bitsquid.blogspot.de/2014/09/building-data-oriented-entity-system.html)
   * [Building a Data-Oriented Entity System (Part 3: The Transform Component)](http://bitsquid.blogspot.de/2014/10/building-data-oriented-entity-system.html)
   * [Building a Data-Oriented Entity System (Part 4: Entity Resources)](http://bitsquid.blogspot.de/2014/10/building-data-oriented-entity-system_10.html)
 * [Mike Acton 'Data-Oriented Design and C++' (CppCon 2014)](https://www.youtube.com/watch?v=rX0ItVEVjHc)
 * [Introduction to Data-Oriented Design (DICE)](http://www.dice.se/wp-content/uploads/2014/12/Introduction_to_Data-Oriented_Design.pdf)
 * [Pitfalls of Object Oriented Programming (Tony Albrecht, Sony)](http://harmful.cat-v.org/software/OO_programming/_pdf/Pitfalls_of_Object_Oriented_Programming_GCAP_09.pdf)