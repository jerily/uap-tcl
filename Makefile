ifndef NAVISERVER
    NAVISERVER  = /usr/local/ns
endif

#
# Module name
#
MOD      =  libtcluap.so

#
# Objects to build.
#
MODOBJS     = src/library.o

#MODLIBS  +=

CFLAGS += -DUSE_NAVISERVER

include  $(NAVISERVER)/include/Makefile.module