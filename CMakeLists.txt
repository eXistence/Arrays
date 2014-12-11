cmake_minimum_required (VERSION 2.8.11)
project (structure-of-arrays)

include_directories(include)

SET(HEADERS
 include/johl/SoA.h
 include/johl/ArrayRef.h
 include/johl/detail/SoA_detail.h
)

SET(SOURCES
  test/main.cpp
)

add_executable("soatest" ${HEADERS} ${SOURCES})