# $Id: makefile,v 1.2 2004/03/27 18:55:45 mihajlov Exp $

export CFLAGS=-Wall -g
LDFLAGS=$(CFLAGS)
CPPFLAGS=
LOADLIBES=

SUBDIRS=
SOURCES=mcserv_unix.c mcclt_unix.c mcstest.c mcctest.c
DEPFILES=$(SOURCES:.c=.d)
OBJECTS=$(SOURCES:.c=.o)

include ../rules

.PHONY: all clean distclean install uninstall $(SUBDIRS) 

all: $(SUBDIRS) libmcserv.so mcstest mcctest

mcstest: LDFLAGS+=-L .
mcstest: LDLIBS=-lmcserv

mcctest: LDFLAGS+=-L .
mcctest: LDLIBS=-lmcserv

libmcserv.so: libmcserv_unix.so
	cp $< $@

libmcserv_unix.so: mcclt_unix.o mcserv_unix.o

clean: $(SUBDIRS)
	$(RM) $(OBJECTS) libmcserv.so libmcserv_unix.so mcstest mcctest 

distclean: clean
	$(RM) $(DEPFILES)

install: all
	/usr/bin/install libmcserv.so $(INSTALLROOT)/usr/lib

uninstall:
	$(RM) $(INSTALLROOT)/usr/lib/libmcserv.so

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