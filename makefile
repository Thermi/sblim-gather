# $Id: makefile,v 1.8 2004/10/19 16:09:08 mihajlov Exp $

CD=cd
export CFLAGS=-Wall -g -fPIC
CPPFLAGS=-Icomms
LDLIBS=-lpthread -ldl

SUBDIRS=comms samples # plugin  provider
SOURCES=mlist.c mretr.c mplugmgr.c mreg.c mrwlock.c mreposl.c mreposp.c \
	gather.c gatherctl.c rgather.c gatherd.c commheap.c \
	rrepos.c repos.c reposctl.c rplugmgr.c rreg.c reposd.c gatherutil.c \
	mcfg.c mcfgtest.c gathercfg.c mlog.c
DEPFILES=$(SOURCES:.c=.d)
OBJECTS=$(SOURCES:.c=.o)
LIBRARIES=libgatherutil.so librrepos.so librepos.so libgather.so librgather.so
EXECUTABLES=gatherd gatherctl reposd reposctl

include rules

.PHONY: all clean distclean install uninstall $(SUBDIRS) 

all: $(SUBDIRS) $(LIBRARIES) $(EXECUTABLES) mcfgtest

libgatherutil.so: gatherutil.o commheap.o mrwlock.o mcfg.o gathercfg.o mlog.o

libgather.so: LDFLAGS=-L .
libgather.so: LOADLIBES=-lrrepos -lgatherutil
libgather.so: mlist.o mretr.o mplugmgr.o mreg.o mreposp.o gather.o

librgather.so: LDFLAGS=-L . -L comms
librgather.so: LOADLIBES=-lmcserv -lgatherutil
librgather.so: rgather.o

librrepos.so: LDFLAGS=-L . -L comms
librrepos.so: LOADLIBES=-lmcserv -lrcserv -lgatherutil
librrepos.so: rrepos.o mreposl.o

librepos.so: LDFLAGS=-L .
librepos.so: LOADLIBES=-lgatherutil
librepos.so: repos.o rplugmgr.o mreposl.o rplugmgr.o rreg.o

gatherd: LOADLIBES=-lgather -lmcserv
gatherd: LDFLAGS=-L . -L comms -Xlinker -rpath-link -Xlinker .
gatherd: gatherd.o

reposd: LOADLIBES=-lrepos -lmcserv -lrcserv
reposd: LDFLAGS=-L . -L comms -Xlinker -rpath-link -Xlinker .
reposd: reposd.o

gatherctl: LOADLIBES=-lrgather -lmcserv -lgatherutil
gatherctl: LDFLAGS=-L . -L comms
gatherctl: gatherctl.o

reposctl: LOADLIBES=-lrrepos -lmcserv -lgatherutil
reposctl: LDFLAGS=-L . -L comms
reposctl: reposctl.o

mcfgtest: mcfgtest.o mcfg.o

clean: $(SUBDIRS)
	$(RM) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES) mcfgtest

distclean: clean
	rcsclean
	$(RM) $(DEPFILES)

install: all
	/usr/bin/install -m755 $(EXECUTABLES)	$(INSTALLROOT)/usr/bin
	/usr/bin/install $(LIBRARIES)		$(INSTALLROOT)/usr/lib
	/usr/bin/install gatherd.conf		$(INSTALLROOT)/etc

uninstall: $(SUBDIRS)
	$(CD) $(INSTALLROOT)/usr/lib && $(RM) $(LIBRARIES)
	$(CD) $(INSTALLROOT)/usr/bin && $(RM) $(EXECUTABLES)
	$(CD) $(INSTALLROOT)/etc && $(RM) gatherd.conf

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
