add_subdirectory(googletest/googletest)

include_directories(../include)
include_directories(googletest/googletest/include)

SET(HEADERS
 ../include/johl/Allocator.h
 ../include/johl/Arrays.h
 ../include/johl/ArrayRef.h
 ../include/johl/detail/Arrays.h
)

IF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  SET(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -std=c++11   -Wall -Wpedantic -Wextra -Wunused-value -Wunused-function -Wunused-variable -Wswitch")
  SET(CMAKE_CXX_FLAGS_DEBUG   "-g -fno-omit-frame-pointer -fsanitize=address")
  SET(CMAKE_CXX_FLAGS_RELEASE "-g -O3")
ELSEIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  SET(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -std=gnu++0x -Wall -Wpedantic -Wextra -Wunused-value -Wunused-function -Wunused-variable -Wswitch")
  SET(CMAKE_CXX_FLAGS_DEBUG   "-g -fno-omit-frame-pointer")
  SET(CMAKE_CXX_FLAGS_RELEASE "-O3")
ELSEIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  SET(CMAKE_CXX_FLAGS "/W4 /EHsc")
  SET(CMAKE_CXX_FLAGS_DEBUG   "/MTd /Od /Zi")
  SET(CMAKE_CXX_FLAGS_RELEASE "/MT /O2")    
ENDIF()

add_executable("arrays_test" ${HEADERS} main.cpp)
target_link_libraries(arrays_test gtest)