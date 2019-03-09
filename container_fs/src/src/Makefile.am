bin_PROGRAMS = fcfuse
fcfuse_SOURCES = fcfuse.c log.c log.h  fcfuse_extra.h fcfuse.h fcfuse_functions.c
AM_CFLAGS = @FUSE_CFLAGS@
LDADD = @FUSE_LIBS@ -lfcontainer -lpthread
