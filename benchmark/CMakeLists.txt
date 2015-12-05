# Build google benchmark
add_subdirectory(benchmark)

include_directories(../include)

IF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  SET(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wpedantic -Wextra -Wunused-value -Wunused-function -Wunused-variable -Wswitch -fno-omit-frame-pointer")
  SET(CMAKE_CXX_FLAGS_DEBUG   "-g")
  SET(CMAKE_CXX_FLAGS_RELEASE "-O3")
ELSEIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  SET(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -std=gnu++0x -Wall -Wpedantic -Wextra -Wunused-value -Wunused-function -Wunused-variable -Wswitch -fno-omit-frame-pointer")
  SET(CMAKE_CXX_FLAGS_DEBUG   "-g")
  SET(CMAKE_CXX_FLAGS_RELEASE "-O3")
ENDIF()

add_executable("arrays_benchmark" main.cpp)
target_link_libraries(arrays_benchmark ${CMAKE_THREAD_LIBS_INIT} benchmark)
