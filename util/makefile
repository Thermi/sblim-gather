# $Id: makefile,v 1.1 2004/10/20 08:15:20 mihajlov Exp $

CD=cd
export CFLAGS=-Wall -g -fPIC
CPPFLAGS=-Icomms
LDLIBS=

SUBDIRS=
SOURCES=mrwlock.c commheap.c gatherutil.c \
	mcfg.c mcfgtest.c gathercfg.c mlog.c
DEPFILES=$(SOURCES:.c=.d)
OBJECTS=$(SOURCES:.c=.o)
LIBRARIES=libgatherutil.so
EXECUTABLES=mcfgtest

include ../rules

.PHONY: all clean distclean install uninstall $(SUBDIRS) 

all: $(SUBDIRS) $(LIBRARIES) $(EXECUTABLES) mcfgtest

libgatherutil.so: gatherutil.o commheap.o mrwlock.o mcfg.o gathercfg.o mlog.o

mcfgtest: mcfgtest.o mcfg.o

clean: $(SUBDIRS)
	$(RM) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)

distclean: clean
	rcsclean
	$(RM) $(DEPFILES)

install: all
	/usr/bin/install $(LIBRARIES)		$(INSTALLROOT)/usr/lib

uninstall: $(SUBDIRS)
	$(CD) $(INSTALLROOT)/usr/lib && $(RM) $(LIBRARIES)

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
