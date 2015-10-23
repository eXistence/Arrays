cmake_minimum_required (VERSION 2.8.11)
project (Arrays)

include_directories(include)

SET(HEADERS
 include/johl/Arrays.h
 include/johl/ArrayRef.h
 include/johl/internal/Arrays_internal.h
)

add_executable("arrays_test" ${HEADERS} test/main.cpp)