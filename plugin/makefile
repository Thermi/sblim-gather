# $Id: makefile,v 1.1 2003/10/17 13:56:01 mihajlov Exp $

# define the Hardware Platform the compile will run on
HW=$(shell ./platform.sh)

export CFLAGS=-Wall -g -D$(HW)
CPPFLAGS=-I ..
LOADLIBES=

SUBDIRS=
SOURCES=metricOperatingSystem.c metricLocalFileSystem.c metricUnixProcess.c metricProcessor.c metricNetworkPort.c
DEPFILES=$(SOURCES:.c=.d)
OBJECTS=$(SOURCES:.c=.o)

include ../rules

.PHONY: all clean distclean install uninstall $(SUBDIRS) 

all: $(SUBDIRS) libmetricOperatingSystem.so libmetricLocalFileSystem.so libmetricUnixProcess.so libmetricProcessor.so libmetricNetworkPort.so

clean: $(SUBDIRS)
	$(RM) $(OBJECTS) libmetric*.so

distclean: clean
	rcsclean
	$(RM) $(DEPFILES)

install: all
	/usr/bin/install libmetricOperatingSystem.so $(INSTALLROOT)/usr/local/lib
	/usr/bin/install libmetricLocalFileSystem.so $(INSTALLROOT)/usr/local/lib
	/usr/bin/install libmetricUnixProcess.so $(INSTALLROOT)/usr/local/lib
	/usr/bin/install libmetricProcessor.so $(INSTALLROOT)/usr/local/lib
	/usr/bin/install libmetricNetworkPort.so $(INSTALLROOT)/usr/local/lib

unistall:
	$(RM) $(INSTALLROOT)/usr/local/lib/libmetricOperatingSystem.so
	$(RM) $(INSTALLROOT)/usr/local/lib/libmetricLocalFileSystem.so
	$(RM) $(INSTALLROOT)/usr/local/lib/libmetricUnixProcess.so
	$(RM) $(INSTALLROOT)/usr/local/lib/libmetricProcessor.so
	$(RM) $(INSTALLROOT)/usr/local/lib/libmetricNetworkPort.so

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
