add_subdirectory(googletest/googletest)

include_directories(../include)
include_directories(googletest/googletest/include)

SET(HEADERS
 ../include/johl/Arrays.h
 ../include/johl/ArrayRef.h
 ../include/johl/internal/Arrays_internal.h
)

IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_FLAGS "-std=c++11")
  SET(CMAKE_CXX_FLAGS_DEBUG   "-O0")
  SET(CMAKE_CXX_FLAGS_RELEASE "-O2")    
ELSEIF(MSVC)  
  SET(CMAKE_CXX_FLAGS_DEBUG   "/MTd /Od /Zi")
  SET(CMAKE_CXX_FLAGS_RELEASE "/MT /O2")  
ENDIF()

add_executable("arrays_test" ${HEADERS} main.cpp)
target_link_libraries(arrays_test gtest)