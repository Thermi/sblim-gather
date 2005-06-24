# $Id: Makefile.am,v 1.1 2005/06/24 12:09:36 mihajlov Exp $
# ==================================================================
# (C) Copyright IBM Corp. 2005
#
# THIS FILE IS PROVIDED UNDER THE TERMS OF THE COMMON PUBLIC LICENSE
# ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
# CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
#
# You can obtain a current copy of the Common Public License from
# http://www.opensource.org/licenses/cpl1.0.php
#
# Author:       Viktor Mihajlovski <mihajlov@de.ibm.com>
# Contributors: Dr. Gareth S. Bestor <bestorga@us.ibm.com>
# Description:  Automake input file for Files and Directories sample provider
# ==================================================================

SUBDIRS=. $(PROVIDER_SUBDIR)

# Start from an empty extra distribution file list
EXTRA_DIST=

# docdir defines where the documentation goes
docdir=$(datadir)/doc/$(PACKAGE)-$(VERSION)

# plugin directories
metricplugindir=$(libdir)
reposplugindir=$(libdir)

#
# Automake instructions for documentation
#
doc_DATA=README AUTHORS COPYING README.TEST

# ADD EXTRA DOC FILES IF PRESENT
doc_DATA+=plugin/metricIPProtocolEndpoint.readme \
	plugin/metricNetworkPort.readme \
	plugin/metricProcessor.readme \
	plugin/metricLocalFileSystem.readme \
	plugin/metricOperatingSystem.readme \
	plugin/metricUnixProcess.readme 
EXTRA_DIST+=$(doc_DATA)

#doc_DATA+=doc/rpmbuild.txt
#EXTRA_DIST+=doc

# Generally useful flags
AM_CPPFLAGS = -I $(srcdir)/util -I $(srcdir)/comms @ARCHDEF@
AM_CFLAGS = -Wall

# Include files
noinst_HEADERS = cimplug.h gatherc.h marshal.h mlist.h mplugmgr.h \
	mrepos.h repos.h rplugin.h rreg.h sforward.h \
	config.h gather.h metric.h mplugin.h mreg.h mretr.h \
	rgather.h rplugmgr.h rrepos.h slisten.h \
	util/commheap.h util/gathercfg.h util/merrdefs.h \
	util/mlog.h util/mtrace.h util/commutil.h util/mcfg.h \
	util/merrno.h util/mrwlock.h util/reposcfg.h \
	comms/mcclt.h comms/mcdefs.h comms/mcserv.h comms/rcclt.h \
	comms/rcserv.h

# Gather support utility libraries
lib_LTLIBRARIES = libgatherutil.la libmcserv.la librcserv.la \
	librrepos.la librepos.la libgather.la librgather.la

libgatherutil_la_SOURCES = util/mrwlock.c util/commheap.c util/commutil.c \
	util/gatherutil.c util/mcfg.c util/gathercfg.c util/reposcfg.c \
	util/mlog.c util/mtrace.c util/merrno.c 
libgatherutil_la_CPPFLAGS = $(AM_CPPFLAGS) -DGATHER_CONFDIR=\"$(sysconfdir)\"

libmcserv_la_SOURCES = comms/mcclt_unix.c comms/mcserv_unix.c
libmcserv_la_LIBADD = -lgatherutil

librcserv_la_SOURCES = comms/rcclt_ip.c comms/rcserv_ip.c
librcserv_la_LIBADD = -lgatherutil

librrepos_la_SOURCES = rrepos.c mreposl.c marshal.c slisten.c
librrepos_la_LIBADD = -lgatherutil -lmcserv -lrcserv

librepos_la_SOURCES = repos.c mreposl.c rplugmgr.c rreg.c marshal.c
librepos_la_LIBADD = -lgatherutil

librgather_la_SOURCES = rgather.c
librgather_la_LIBADD = -lgatherutil -lmcserv

libgather_la_SOURCES = mlist.c mretr.c mplugmgr.c mreg.c mreposp.c gather.c
libgather_la_LIBADD = -lrrepos -lgatherutil

# Gather daemons
sbin_PROGRAMS = gatherd reposd

gatherd_SOURCES = gatherd.c
gatherd_LDADD = -lgather -lmcserv

reposd_SOURCES = reposd.c sforward.c
reposd_LDADD = -lrepos -lmcserv -lrcserv

# Gather CLIs
bin_PROGRAMS = gatherctl reposctl

gatherctl_SOURCES = gatherctl.c
gatherctl_LDADD = -lrgather

reposctl_SOURCES = reposctl.c
reposctl_LDADD = -lrrepos


# Gatherer Plugins
metricplugin_LTLIBRARIES = libmetricOperatingSystem.la \
	libmetricLocalFileSystem.la \
	libmetricUnixProcess.la \
	libmetricProcessor.la \
	libmetricNetworkPort.la \
	libmetricIPProtocolEndpoint.la 

libmetricOperatingSystem_la_SOURCES=plugin/metricOperatingSystem.c
libmetricOperatingSystem_la_LIBADD=-lgatherutil

libmetricLocalFileSystem_la_SOURCES=plugin/metricLocalFileSystem.c
libmetricLocalFileSystem_la_LIBADD=-lgatherutil

libmetricUnixProcess_la_SOURCES=plugin/metricUnixProcess.c

libmetricProcessor_la_SOURCES=plugin/metricProcessor.c
libmetricProcessor_la_LIBADD=-lgatherutil

libmetricNetworkPort_la_SOURCES=plugin/metricNetworkPort.c

libmetricIPProtocolEndpoint_la_SOURCES=plugin/metricIPProtocolEndpoint.c

reposplugin_LTLIBRARIES = librepositoryOperatingSystem.la \
	librepositoryLocalFileSystem.la \
	librepositoryUnixProcess.la \
	librepositoryProcessor.la \
	librepositoryNetworkPort.la \
	librepositoryIPProtocolEndpoint.la 

librepositoryOperatingSystem_la_SOURCES=plugin/repositoryOperatingSystem.c
librepositoryOperatingSystem_la_LIBADD=-lgatherutil

librepositoryLocalFileSystem_la_SOURCES=plugin/repositoryLocalFileSystem.c
librepositoryLocalFileSystem_la_LIBADD=-lgatherutil

librepositoryUnixProcess_la_SOURCES=plugin/repositoryUnixProcess.c

librepositoryProcessor_la_SOURCES=plugin/repositoryProcessor.c
librepositoryProcessor_la_LIBADD=-lgatherutil

librepositoryNetworkPort_la_SOURCES=plugin/repositoryNetworkPort.c

librepositoryIPProtocolEndpoint_la_SOURCES=plugin/repositoryIPProtocolEndpoint.c

# Gather utility test programs (not installed)
noinst_PROGRAMS = mcfgtest mtracetest merrnotest mcstest mcctest \
	rcstest rcctest

mcfgtest_SOURCES = util/mcfgtest.c
mcfgtest_LDADD = -lgatherutil

mtracetest_SOURCES = util/mtracetest.c
mtracetest_LDADD = -lgatherutil

merrnotest_SOURCES = util/merrnotest.c
merrnotest_LDADD = -lgatherutil

mcstest_SOURCES = comms/mcstest.c
mcstest_LDADD = -lmcserv

mcctest_SOURCES = comms/mcctest.c
mcctest_LDADD = -lmcserv

rcstest_SOURCES = comms/rcstest.c
rcstest_LDADD = -lrcserv

rcctest_SOURCES = comms/rcctest.c
rcctest_LDADD = -lrcserv

EXTRA_DIST += util/test.cfg

# Gather config files
sysconf_DATA = gatherd.conf reposd.conf
EXTRA_DIST += $(sysconf_DATA)

# We must explicity add the RPM spec file to the distribution package
#EXTRA_DIST+=sblim-cmpi-fad.spec

EXTRA_DIST += samples

postinstall:
	$(MAKE) -C $(PROVIDER_SUBDIR) postinstall

preuninstall:
	$(MAKE) -C $(PROVIDER_SUBDIR) preuninstall

dist-hook:
	test -d "$(distdir)" &&	rm -rf `find $(distdir) -type d -name CVS`
