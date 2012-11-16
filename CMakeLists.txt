
cmake_minimum_required(VERSION 2.6)

project(Iniread)

include_directories(include)

add_subdirectory(src)
add_subdirectory(tests)

install ( FILES "include/iniread.h"
          DESTINATION include
        )
