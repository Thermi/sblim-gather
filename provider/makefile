# $Id: makefile,v 1.1 2003/10/17 13:56:01 mihajlov Exp $

include env

export CFLAGS=-Wall -g 
CPPFLAGS=-I .. -I $(CMPIINCL)
LOADLIBES=-L .. -lrgather

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

libOSBase_MetricGathererProvider.so: LDLIBS=-L $(CMPILIB) -lcmpiOSBase_Common

clean: $(SUBDIRS)
	$(RM) $(OBJECTS) libOSBase_Metric*.so

distclean: clean
	rcsclean
	$(RM) $(DEPFILES)

install: all 
	install libOSBase_Metric*.so $(CMPILIB)

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