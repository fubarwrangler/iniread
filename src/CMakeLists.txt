add_library(iniread SHARED iniread.c)

INSTALL (   TARGETS iniread
            DESTINATION lib
        )
