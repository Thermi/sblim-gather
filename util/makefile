# $Id: makefile,v 1.4 2004/10/21 15:51:18 mihajlov Exp $

CD=cd
export CFLAGS=-Wall -g -fPIC
CPPFLAGS=-I.
LDFLAGS=$(CFLAGS)
LDLIBS=-lpthread

SUBDIRS=
SOURCES=mrwlock.c commheap.c gatherutil.c \
	mcfg.c mcfgtest.c gathercfg.c reposcfg.c mlog.c mtrace.c \
	mtracetest.c merrno.c merrnotest.c

DEPFILES=$(SOURCES:.c=.d)
OBJECTS=$(SOURCES:.c=.o)
LIBRARIES=libgatherutil.so
EXECUTABLES=mcfgtest mtracetest merrnotest

include ../rules

.PHONY: all clean distclean install uninstall $(SUBDIRS) 

all: $(SUBDIRS) $(LIBRARIES) $(EXECUTABLES)

libgatherutil.so: gatherutil.o commheap.o mrwlock.o mcfg.o gathercfg.o \
		reposcfg.o mlog.o mtrace.o merrno.o

mcfgtest: mcfgtest.o mcfg.o mlog.o

mtracetest: mtracetest.o mtrace.o

merrnotest: merrnotest.o merrno.o

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
