
add_library(iniread SHARED iniread.c hash.c)

target_link_libraries(iniread m)
