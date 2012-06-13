add_library(iniread SHARED iniread.c readline.c hash.c interpolate.c)

target_link_libraries(iniread m)
