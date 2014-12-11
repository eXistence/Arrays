cmake_minimum_required (VERSION 2.8.11)
project (structure-of-arrays)

include_directories(include)

SET(HEADERS
 include/SoA.h
 include/ArrayRef.h
 include/detail/SoA_detail.h
)

SET(SOURCES
  test/main.cpp
)

add_executable("soatest" ${HEADERS} ${SOURCES})