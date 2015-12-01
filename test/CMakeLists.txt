add_subdirectory(googletest/googletest)

include_directories(../include)
include_directories(googletest/googletest/include)

SET(HEADERS
 ../include/johl/utils.h
 ../include/johl/Arrays.h
 ../include/johl/ArrayRef.h
 ../include/johl/internal/Arrays_internal.h
)

SET(COMPILER_IS_CLANG "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

IF(CMAKE_COMPILER_IS_GNUCXX OR COMPILER_IS_CLANG)
  SET(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wpedantic -Wextra")

  SET(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g -fno-omit-frame-pointer")
  SET(CMAKE_CXX_FLAGS_RELEASE "-O2") 

  IF (COMPILER_IS_CLANG)
  	SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fno-omit-frame-pointer -fsanitize=address")
  ENDIF()
ELSEIF(MSVC)  
  SET(CMAKE_CXX_FLAGS_DEBUG   "/MTd /Od /Zi")
  SET(CMAKE_CXX_FLAGS_RELEASE "/MT /O2")  
ENDIF()

add_executable("arrays_test" ${HEADERS} main.cpp)
target_link_libraries(arrays_test gtest)