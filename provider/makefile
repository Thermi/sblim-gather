# $Id: makefile,v 1.9 2004/10/07 06:22:00 mihajlov Exp $

include setting.cmpi

export CFLAGS=-Wall -g 
CPPFLAGS=-I .. -I $(CIMOMINC) -I $(COMMONINC)
LOADLIBES=-L .. -lrrepos -lgatherutil -L . -lOSBase_MetricUtil

SUBDIRS=mof
SOURCES=OSBase_MetricDefinitionProvider.c OSBase_MetricValueProvider.c \
	OSBase_MetricGathererProvider.c \
	OSBase_MetricRepositoryServiceProvider.c \
	OSBase_MetricInstanceProvider.c \
	OSBase_MetricForMEProvider.c \
	OSBase_MetricUtil.c
DEPFILES=$(SOURCES:.c=.d)
OBJECTS=$(SOURCES:.c=.o)

include ../rules

.PHONY: all clean distclean $(SUBDIRS) 

all: $(SUBDIRS) 	libOSBase_MetricUtil.so \
	libOSBase_MetricDefinitionProvider.so \
	libOSBase_MetricValueProvider.so \
	libOSBase_MetricGathererProvider.so \
	libOSBase_MetricRepositoryServiceProvider.so \
	libOSBase_MetricForMEProvider.so \
	libOSBase_MetricInstanceProvider.so

libOSBase_MetricGathererProvider.so: LOADLIBES=-L .. -lrgather -lcmpiOSBase_Common
libOSBase_MetricRepositoryServiceProvider.so: LDLIBS=-lcmpiOSBase_Common

libOSBase_MetricUtil.so: LOADLIBES=-L .. -lrrepos -lgatherutil

clean: $(SUBDIRS)
	$(RM) $(OBJECTS) libOSBase_Metric*.so

distclean: clean
	rcsclean
	$(RM) $(DEPFILES)

install: all 
	install libOSBase_Metric*Provider.so $(CIMOMLIB)
	install libOSBase_MetricUtil.so $(COMMONLIB)

uninstall: all
	$(RM) $(CIMOMLIB)/libOSBase_Metric*.so
	$(RM) $(COMMONLIB)/libOSBase_Metric*.so

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
