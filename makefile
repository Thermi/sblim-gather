# $Id: makefile,v 1.10 2004/10/20 08:53:56 heidineu Exp $

CD=cd
export CFLAGS=-Wall -g -fPIC
CPPFLAGS=-Icomms -Iutil
LDLIBS=-lpthread -ldl

SUBDIRS=util comms samples # plugin  provider
SOURCES=mlist.c mretr.c mplugmgr.c mreg.c mreposl.c mreposp.c \
	gather.c gatherctl.c rgather.c gatherd.c \
	rrepos.c repos.c reposctl.c rplugmgr.c rreg.c reposd.c 

DEPFILES=$(SOURCES:.c=.d)
OBJECTS=$(SOURCES:.c=.o)
LIBRARIES=librrepos.so librepos.so libgather.so librgather.so
EXECUTABLES=gatherd gatherctl reposd reposctl

include rules

.PHONY: all clean distclean install uninstall $(SUBDIRS) 

all: $(SUBDIRS) $(LIBRARIES) $(EXECUTABLES)

libgather.so: LDFLAGS=-L . -L util
libgather.so: LOADLIBES=-lrrepos -lgatherutil
libgather.so: mlist.o mretr.o mplugmgr.o mreg.o mreposp.o gather.o

librgather.so: LDFLAGS=-L . -L comms -L util
librgather.so: LOADLIBES=-lmcserv -lgatherutil
librgather.so: rgather.o

librrepos.so: LDFLAGS=-L . -L comms -L util
librrepos.so: LOADLIBES=-lmcserv -lrcserv -lgatherutil
librrepos.so: rrepos.o mreposl.o

librepos.so: LDFLAGS=-L . -L util
librepos.so: LOADLIBES=-lgatherutil
librepos.so: repos.o rplugmgr.o mreposl.o rplugmgr.o rreg.o

gatherd: LOADLIBES=-lgather -lmcserv
gatherd: LDFLAGS=-L . -L comms -Xlinker -rpath-link -Xlinker .
gatherd: gatherd.o

reposd: LOADLIBES=-lrepos -lmcserv -lrcserv
reposd: LDFLAGS=-L . -L comms -Xlinker -rpath-link -Xlinker .
reposd: reposd.o

gatherctl: LOADLIBES=-lrgather -lmcserv -lgatherutil
gatherctl: LDFLAGS=-L . -L comms -L util
gatherctl: gatherctl.o

reposctl: LOADLIBES=-lrrepos -lmcserv -lgatherutil
reposctl: LDFLAGS=-L . -L comms -L util
reposctl: reposctl.o

clean: $(SUBDIRS)
	$(RM) $(OBJECTS) $(LIBRARIES) $(EXECUTABLES)

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
