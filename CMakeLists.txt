
cmake_minimum_required(VERSION 2.6)

project(Iniread)

include_directories(include)

add_subdirectory(src)
add_subdirectory(tests)

set(CMAKE_C_FLAGS -Wall -std=c99 -Wpedantic)
