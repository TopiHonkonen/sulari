
cmake_minimum_required(VERSION 3.24.0)

set (This TimeParser)

project(${This} CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

enable_testing()

add_subdirectory(googletest)

set(Headers
	src/time_parse.h
)
set(Sources
	src/time_parse.c
)

add_library(${This} STATIC ${Sources} ${Headers})

add_subdirectory(test_cases)


