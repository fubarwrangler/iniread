SET(INI_SRC iniread.c readline.c types.c)
add_library(iniread ${INI_SRC})
INSTALL (   TARGETS iniread
            DESTINATION lib
        )
