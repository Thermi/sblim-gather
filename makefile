# $Id: makefile,v 1.1 2003/10/17 13:56:01 mihajlov Exp $

export CFLAGS=-Wall -g 
CPPFLAGS=-Icomms
LOADLIBES=-lpthread -ldl

SUBDIRS=comms samples plugin # provider
SOURCES=mlist.c mretr.c mplugmgr.c mreg.c mrwlock.c mreposl.c \
	gather.c gatherctl.c rgather.c gatherd.c commheap.c
DEPFILES=$(SOURCES:.c=.d)
OBJECTS=$(SOURCES:.c=.o)

include rules

.PHONY: all clean distclean install uninstall $(SUBDIRS) 

all: $(SUBDIRS) libgather.so librgather.so gatherd gatherctl

libgather.so: mlist.o mretr.o mplugmgr.o mreg.o mrwlock.o mreposl.o gather.o commheap.o

librgather.so: LDFLAGS=-L comms
librgather.so: LOADLIBES=-lmcserv
librgather.so: rgather.o commheap.o

gatherd: LOADLIBES=-lgather -lmcserv
gatherd: LDFLAGS=-L . -L comms

gatherctl: LOADLIBES=-lrgather -lmcserv
gatherctl: LDFLAGS=-L . -L comms

clean: $(SUBDIRS)
	$(RM) $(OBJECTS) gatherd gatherctl libgather.so librgather.so

distclean: clean
	rcsclean
	$(RM) $(DEPFILES)

install: all
	/usr/bin/install -m755 gatherd   $(INSTALLROOT)/usr/local/bin
	/usr/bin/install -m755 gatherctl $(INSTALLROOT)/usr/local/bin
	/usr/bin/install libgather.so    $(INSTALLROOT)/usr/local/lib
	/usr/bin/install librgather.so   $(INSTALLROOT)/usr/local/lib

uninstall: $(SUBDIRS)
	$(RM) $(INSTALLROOT)/usr/local/lib/librgather.so \
		$(INSTALLROOT)/usr/local/lib/libgather.so \
		$(INSTALLROOT)/usr/local/bin/gatherd \
		$(INSTALLROOT)/usr/local/bin/gatherctl

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
