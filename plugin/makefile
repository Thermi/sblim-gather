# $Id: makefile,v 1.8 2004/10/08 07:34:54 mihajlov Exp $

# define the Hardware Platform the compile will run on
HW=$(shell ./platform.sh)

export CFLAGS=-Wall -g
CPPFLAGS=-I .. -D$(HW) -I$(PEGASUS_ROOT)/src/Pegasus/Provider/CMPI
LOADLIBES=

SUBDIRS=
SOURCES= metricOperatingSystem.c repositoryOperatingSystem.c \
	 cimplugOperatingSystem.c \
	 metricLocalFileSystem.c repositoryLocalFileSystem.c \
	 cimplugLocalFileSystem.c \
	 metricUnixProcess.c repositoryUnixProcess.c \
	 cimplugUnixProcess.c \
	 metricProcessor.c repositoryProcessor.c \
	 cimplugProcessor.c \
	 metricNetworkPort.c repositoryNetworkPort.c \
	 cimplugNetworkPort.c \
	 metricIPProtocolEndpoint.c repositoryIPProtocolEndpoint.c \
	 cimplugIPProtocolEndpoint.c

DEPFILES=$(SOURCES:.c=.d)
OBJECTS=$(SOURCES:.c=.o)

include ../rules

.PHONY: all clean distclean install uninstall $(SUBDIRS) 

all: $(SUBDIRS) libmetricOperatingSystem.so librepositoryOperatingSystem.so \
	libcimplugOperatingSystem.so \
	libmetricLocalFileSystem.so librepositoryLocalFileSystem.so \
	libcimplugLocalFileSystem.so \
	libmetricUnixProcess.so librepositoryUnixProcess.so \
	libcimplugUnixProcess.so \
	libmetricProcessor.so librepositoryProcessor.so \
	libcimplugProcessor.so \
	libmetricNetworkPort.so librepositoryNetworkPort.so \
	libcimplugNetworkPort.so \
	libmetricIPProtocolEndpoint.so librepositoryIPProtocolEndpoint.so \
	libcimplugIPProtocolEndpoint.so


clean: $(SUBDIRS)
	$(RM) $(OBJECTS) libmetric*.so librepository*.so libcimplug*.so

distclean: clean
	rcsclean
	$(RM) $(DEPFILES)

install: all
	/usr/bin/install libmetricOperatingSystem.so $(INSTALLROOT)/usr/lib
	/usr/bin/install librepositoryOperatingSystem.so $(INSTALLROOT)/usr/lib
	/usr/bin/install libcimplugOperatingSystem.so $(INSTALLROOT)/usr/lib
	/usr/bin/install libmetricLocalFileSystem.so $(INSTALLROOT)/usr/lib
	/usr/bin/install librepositoryLocalFileSystem.so $(INSTALLROOT)/usr/lib
	/usr/bin/install libcimplugLocalFileSystem.so $(INSTALLROOT)/usr/lib
	/usr/bin/install libmetricUnixProcess.so $(INSTALLROOT)/usr/lib
	/usr/bin/install librepositoryUnixProcess.so $(INSTALLROOT)/usr/lib
	/usr/bin/install libcimplugUnixProcess.so $(INSTALLROOT)/usr/lib
	/usr/bin/install libmetricProcessor.so $(INSTALLROOT)/usr/lib
	/usr/bin/install librepositoryProcessor.so $(INSTALLROOT)/usr/lib
	/usr/bin/install libcimplugProcessor.so $(INSTALLROOT)/usr/lib
	/usr/bin/install libmetricNetworkPort.so $(INSTALLROOT)/usr/lib
	/usr/bin/install librepositoryNetworkPort.so $(INSTALLROOT)/usr/lib
	/usr/bin/install libcimplugNetworkPort.so $(INSTALLROOT)/usr/lib
	/usr/bin/install libmetricIPProtocolEndpoint.so $(INSTALLROOT)/usr/lib
	/usr/bin/install librepositoryIPProtocolEndpoint.so $(INSTALLROOT)/usr/lib
	/usr/bin/install libcimplugIPProtocolEndpoint.so $(INSTALLROOT)/usr/lib

unistall:
	$(RM) $(INSTALLROOT)/usr/lib/libmetricOperatingSystem.so
	$(RM) $(INSTALLROOT)/usr/lib/librepositoryOperatingSystem.so
	$(RM) $(INSTALLROOT)/usr/lib/libcimplugOperatingSystem.so
	$(RM) $(INSTALLROOT)/usr/lib/libmetricLocalFileSystem.so
	$(RM) $(INSTALLROOT)/usr/lib/librepositoryLocalFileSystem.so
	$(RM) $(INSTALLROOT)/usr/lib/libcimplugLocalFileSystem.so
	$(RM) $(INSTALLROOT)/usr/lib/libmetricUnixProcess.so
	$(RM) $(INSTALLROOT)/usr/lib/librepositoryUnixProcess.so
	$(RM) $(INSTALLROOT)/usr/lib/libcimplugUnixProcess.so
	$(RM) $(INSTALLROOT)/usr/lib/libmetricProcessor.so
	$(RM) $(INSTALLROOT)/usr/lib/librepositoryProcessor.so
	$(RM) $(INSTALLROOT)/usr/lib/libcimplugProcessor.so
	$(RM) $(INSTALLROOT)/usr/lib/libmetricNetworkPort.so
	$(RM) $(INSTALLROOT)/usr/lib/librepositoryNetworkPort.so
	$(RM) $(INSTALLROOT)/usr/lib/libcimplugNetworkPort.so
	$(RM) $(INSTALLROOT)/usr/lib/libmetricIPProtocolEndpoint.so
	$(RM) $(INSTALLROOT)/usr/lib/librepositoryIPProtocolEndpoint.so
	$(RM) $(INSTALLROOT)/usr/lib/libcimplugIPProtocolEndpoint.so

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
