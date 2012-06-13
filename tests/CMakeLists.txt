add_executable(initest initest.c)
target_link_libraries(initest iniread)

add_executable(interp interp.c)
target_link_libraries(interp iniread)
