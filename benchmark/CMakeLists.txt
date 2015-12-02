# Build google benchmark
add_subdirectory(benchmark)

include_directories(../include)

IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  SET(COMPILER_IS_CLANG TRUE)
ELSE()
  SET(COMPILER_IS_CLANG FALSE)
ENDIF()

IF(CMAKE_COMPILER_IS_GNUCXX OR COMPILER_IS_CLANG)
  SET(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wpedantic -Wextra")
  SET(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g -fno-omit-frame-pointer")
  SET(CMAKE_CXX_FLAGS_RELEASE "-O2 -g -fno-omit-frame-pointer")
ELSEIF(MSVC)  
  SET(CMAKE_CXX_FLAGS "/W4 /EHsc")
  SET(CMAKE_CXX_FLAGS_DEBUG   "/MTd /Od /Zi")
  SET(CMAKE_CXX_FLAGS_RELEASE "/MT /O2")  
ENDIF()


add_executable("arrays_benchmark" main.cpp)
target_link_libraries(arrays_benchmark ${CMAKE_THREAD_LIBS_INIT} benchmark)
