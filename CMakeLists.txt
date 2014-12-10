cmake_minimum_required (VERSION 2.8.11)
project (structure-of-arrays)

SET(HEADERS
 include/SoA.h
 include/details/SoA_detail.h
)

SET(SOURCES
  test/main.cpp
)

add_executable("soatest", ${HEADERS} ${SOURCES})