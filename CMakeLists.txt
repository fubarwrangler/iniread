
cmake_minimum_required(VERSION 2.6)

add_definitions("-std=c99 -Wall -Wpedantic")

project(Iniread)

include_directories(include)
if(WITH_STATIC)
    set(LIBINIREAD_STATIC 1)
endif()

add_subdirectory(src)
add_subdirectory(tests)

install ( FILES "include/iniread.h"
          DESTINATION include
        )

