# $Id: makefile,v 1.5 2004/08/04 07:29:26 mihajlov Exp $

include setting.cmpi

export CFLAGS=-Wall -g 
CPPFLAGS=-I .. -I $(CIMOMINC) -I $(COMMONINC)
LOADLIBES=-L .. -lrrepos -lgatherutil

SUBDIRS=mof
SOURCES=OSBase_MetricDefinitionProvider.c OSBase_MetricValueProvider.c \
	OSBase_MetricGathererProvider.c
DEPFILES=$(SOURCES:.c=.d)
OBJECTS=$(SOURCES:.c=.o)

include ../rules

.PHONY: all clean distclean $(SUBDIRS) 

all: $(SUBDIRS) libOSBase_MetricDefinitionProvider.so \
	libOSBase_MetricValueProvider.so \
	libOSBase_MetricGathererProvider.so	

libOSBase_MetricGathererProvider.so: LOADLIBES=-L .. -lrgather -lcmpiOSBase_Common

clean: $(SUBDIRS)
	$(RM) $(OBJECTS) libOSBase_Metric*.so

distclean: clean
	rcsclean
	$(RM) $(DEPFILES)

install: all 
	install libOSBase_Metric*.so $(CIMOMLIB)

uninstall: all
	$(RM) $(CMPILIB)/libOSBase_Metric*.so

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
