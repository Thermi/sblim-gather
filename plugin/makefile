# $Id: makefile,v 1.3 2004/08/03 10:24:17 heidineu Exp $

# define the Hardware Platform the compile will run on
HW=$(shell ./platform.sh)

# export CFLAGS=-Wall -g
CPPFLAGS=-I .. -D$(HW) 
LOADLIBES=

SUBDIRS=
SOURCES=metricOperatingSystem.c repositoryOperatingSystem.c # metricLocalFileSystem.c metricUnixProcess.c metricProcessor.c metricNetworkPort.c
DEPFILES=$(SOURCES:.c=.d)
OBJECTS=$(SOURCES:.c=.o)

include ../rules

.PHONY: all clean distclean install uninstall $(SUBDIRS) 

all: $(SUBDIRS) libmetricOperatingSystem.so librepositoryOperatingSystem.so #libmetricLocalFileSystem.so libmetricUnixProcess.so libmetricProcessor.so libmetricNetworkPort.so

clean: $(SUBDIRS)
	$(RM) $(OBJECTS) libmetric*.so librepository*.so

distclean: clean
	rcsclean
	$(RM) $(DEPFILES)

install: all
	/usr/bin/install libmetricOperatingSystem.so $(INSTALLROOT)/usr/lib
	/usr/bin/install librepositoryOperatingSystem.so $(INSTALLROOT)/usr/lib
#	/usr/bin/install libmetricLocalFileSystem.so $(INSTALLROOT)/usr/lib
#	/usr/bin/install libmetricUnixProcess.so $(INSTALLROOT)/usr/lib
#	/usr/bin/install libmetricProcessor.so $(INSTALLROOT)/usr/lib
#	/usr/bin/install libmetricNetworkPort.so $(INSTALLROOT)/usr/lib

unistall:
	$(RM) $(INSTALLROOT)/usr/lib/libmetricOperatingSystem.so
	$(RM) $(INSTALLROOT)/usr/lib/librepositoryOperatingSystem.so
#	$(RM) $(INSTALLROOT)/usr/lib/libmetricLocalFileSystem.so
#	$(RM) $(INSTALLROOT)/usr/lib/libmetricUnixProcess.so
#	$(RM) $(INSTALLROOT)/usr/lib/libmetricProcessor.so
#	$(RM) $(INSTALLROOT)/usr/lib/libmetricNetworkPort.so

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
