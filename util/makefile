# $Id: makefile,v 1.2 2004/10/20 12:49:15 mihajlov Exp $

CD=cd
export CFLAGS=-Wall -g -fPIC
CPPFLAGS=-Icomms
LDFLAGS=$(CFLAGS)
LDLIBS=-lpthread

SUBDIRS=
SOURCES=mrwlock.c commheap.c gatherutil.c \
	mcfg.c mcfgtest.c gathercfg.c mlog.c mtrace.c \
	mtracetest.c

DEPFILES=$(SOURCES:.c=.d)
OBJECTS=$(SOURCES:.c=.o)
LIBRARIES=libgatherutil.so
EXECUTABLES=mcfgtest mtracetest

include ../rules

.PHONY: all clean distclean install uninstall $(SUBDIRS) 

all: $(SUBDIRS) $(LIBRARIES) $(EXECUTABLES) mcfgtest

libgatherutil.so: gatherutil.o commheap.o mrwlock.o mcfg.o gathercfg.o mlog.o \
		mtrace.o

mcfgtest: mcfgtest.o mcfg.o

mtracetest: mtracetest.o mtrace.o

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
