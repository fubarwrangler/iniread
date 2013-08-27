SET(INI_SRC iniread.c readline.c types.c hash.c)
add_library(iniread ${INI_SRC})
INSTALL (   TARGETS iniread
            DESTINATION lib
        )
target_link_libraries(iniread m)
