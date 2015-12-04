cmake_minimum_required (VERSION 2.8.11)
project (Arrays)

add_subdirectory(test)

IF(UNIX)
  add_subdirectory(benchmark)
ENDIF()
