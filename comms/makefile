# $Id: makefile,v 1.5 2004/10/14 14:19:33 heidineu Exp $

export CFLAGS=-Wall -g -fPIC
#export CFLAGS=-Wall -O3 -fPIC
LDFLAGS=$(CFLAGS)
CPPFLAGS=
LOADLIBES=

SUBDIRS=
SOURCES=mcserv_unix.c mcclt_unix.c mcstest.c mcctest.c \
	rcserv_ip.c rcclt_ip.c rcstest.c rcctest.c
DEPFILES=$(SOURCES:.c=.d)
OBJECTS=$(SOURCES:.c=.o)

include ../rules

.PHONY: all clean distclean install uninstall $(SUBDIRS) 

all: $(SUBDIRS) libmcserv.so mcstest mcctest \
	 librcserv.so rcstest rcctest


# Local Communication via UNIX Domain sockets ----------------------

mcstest: LDFLAGS+=-L .
mcstest: LDLIBS=-lmcserv

mcctest: LDFLAGS+=-L .
mcctest: LDLIBS=-lmcserv

libmcserv.so: libmcserv_unix.so
	cp $< $@

libmcserv_unix.so: mcclt_unix.o mcserv_unix.o


# Remote Communication via TCP/IP Sockets --------------------------

rcstest: LDFLAGS+=-L .
rcstest: LDLIBS=-lrcserv -lpthread 

rcctest: LDFLAGS+=-L .
rcctest: LDLIBS=-lrcserv

librcserv.so: librcserv_ip.so
	cp $< $@

librcserv_ip.so: rcclt_ip.o rcserv_ip.o

# ------------------------------------------------------------------

clean: $(SUBDIRS)
	$(RM) $(OBJECTS) libmcserv.so libmcserv_unix.so mcstest mcctest \
	 librcserv.so librcserv_ip.so rcstest rcctest 

distclean: clean
	$(RM) $(DEPFILES)

install: all
	/usr/bin/install libmcserv.so $(INSTALLROOT)/usr/lib
	/usr/bin/install librcserv.so $(INSTALLROOT)/usr/lib

uninstall:
	$(RM) $(INSTALLROOT)/usr/lib/libmcserv.so
	$(RM) $(INSTALLROOT)/usr/lib/librcserv.so

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

ifeq ($(MAKECMDGOALS),clean)
NODEPS=1
endif

ifeq ($(MAKECMDGOALS),distclean)
NODEPS=1
endif

ifndef NODEPS
include $(DEPFILES)
endif
