add_executable(initest initest.c)
target_link_libraries(initest iniread)

add_executable(compare compare.c)
target_link_libraries(compare iniread)
