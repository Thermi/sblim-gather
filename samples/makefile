# $Id: makefile,v 1.2 2004/03/27 18:55:46 mihajlov Exp $

export CFLAGS=-Wall -g 
CPPFLAGS=-I ..
LOADLIBES=

SUBDIRS=
SOURCES=metricSample1.c metricSample2.c metricSample3.c metricSample4.c
DEPFILES=$(SOURCES:.c=.d)
OBJECTS=$(SOURCES:.c=.o)

include ../rules

.PHONY: all clean distclean install uninstall $(SUBDIRS) 

all: $(SUBDIRS) libmetricSample1.so libmetricSample2.so libmetricSample3.so \
 libmetricSample4.so

clean: $(SUBDIRS)
	$(RM) $(OBJECTS) libmetricSample*.so

distclean: clean
	rcsclean
	$(RM) $(DEPFILES)

install: all
#	/usr/bin/install libmetricSample?.so $(INSTALLROOT)/usr/lib

uninstall:
#	$(RM) $(INSTALLROOT)/usr/lib/libmetricSample?.so

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