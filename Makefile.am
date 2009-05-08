# $Id: Makefile.am,v 1.13 2009/05/08 04:44:07 tyreld Exp $
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
#               Oliver Benke <benke@de.ibm.com>
# Description:  Automake input file for Files and Directories sample provider
# ==================================================================

SUBDIRS=. $(PROVIDER_SUBDIR)

# Start from an empty extra distribution file list
EXTRA_DIST=

# gatherer run dir
gatherrundir=$(localstatedir)/run/gather

# docdir defines where the documentation goes
docdir=$(datadir)/doc/$(PACKAGE)-$(VERSION)

# plugin directories
metricplugindir=$(libdir)/gather/mplug
reposplugindir=$(libdir)/gather/rplug

# development headers
pkgincludedir=$(includedir)/gather

#
# Automake instructions for documentation
#
doc_DATA=README AUTHORS COPYING README.TEST reposd2csv.pl

# ADD EXTRA DOC FILES IF PRESENT
doc_DATA+=plugin/metricIPProtocolEndpoint.readme \
	plugin/metricNetworkPort.readme \
	plugin/metricProcessor.readme \
	plugin/metricLocalFileSystem.readme \
	plugin/metricOperatingSystem.readme \
	plugin/metricUnixProcess.readme \
	plugin/metricXen.readme \
	plugin/metricKvm.readme \
	plugin/metriczECKD.readme \
	plugin/metriczCEC.readme \
	plugin/metriczLPAR.readme \
	plugin/metriczCH.readme
EXTRA_DIST+=$(doc_DATA)

#doc_DATA+=doc/rpmbuild.txt
#EXTRA_DIST+=doc

# Generally useful flags
AM_CPPFLAGS = -I $(srcdir)/util -I $(srcdir)/comms @ARCHDEF@
AM_CFLAGS = -Wall

# Include files
pkginclude_HEADERS = cimplug.h metric.h mplugin.h util/commutil.h 
nodist_pkginclude_HEADERS = gather-config.h

noinst_HEADERS = gatherc.h marshal.h mlist.h mplugmgr.h \
	mrepos.h repos.h rplugin.h rreg.h sforward.h \
	gather.h mplugin.h mreg.h mretr.h \
	rgather.h rplugmgr.h rrepos.h slisten.h \
	util/commheap.h util/gathercfg.h util/merrdefs.h \
	util/mlog.h util/mtrace.h util/mcfg.h \
	util/merrno.h util/mrwlock.h util/reposcfg.h util/dirutil.h \
	comms/mcclt.h comms/mcdefs.h comms/mcserv.h comms/rcclt.h \
	comms/rcserv.h \
	plugin/channelutil.h plugin/hypfs.h

# Gather support utility libraries
lib_LTLIBRARIES = libgatherutil.la libmcserv.la librcserv.la \
	librrepos.la librepos.la libgather.la librgather.la \
	libsysfswrapper.la libhypfs.la liblparutil.la libchannelutil.la

libgatherutil_la_SOURCES = util/mrwlock.c util/commheap.c util/commutil.c \
	util/gatherutil.c util/mcfg.c util/gathercfg.c util/reposcfg.c \
	util/mlog.c util/mtrace.c util/merrno.c util/dirutil.c
libgatherutil_la_CPPFLAGS = $(AM_CPPFLAGS) -DGATHER_CONFDIR=\"$(sysconfdir)\"

libmcserv_la_SOURCES = comms/mcclt_unix.c comms/mcserv_unix.c
libmcserv_la_CPPFLAGS = $(AM_CPPFLAGS) -DGATHER_RUNDIR=\"$(gatherrundir)\" 
libmcserv_la_LIBADD = -lgatherutil
libmcserv_la_DEPENDENCIES = libgatherutil.la

librcserv_la_SOURCES = comms/rcclt_ip.c comms/rcserv_ip.c
librcserv_la_LIBADD = -lgatherutil
librcserv_la_DEPENDENCIES = libgatherutil.la

librrepos_la_SOURCES = rrepos.c marshal.c slisten.c
librrepos_la_LIBADD = -lgatherutil -lmcserv -lrcserv
librrepos_la_DEPENDENCIES = libgatherutil.la libmcserv.la librcserv.la

librepos_la_SOURCES = repos.c mreposl.c rplugmgr.c rreg.c marshal.c
librepos_la_CPPFLAGS = $(AM_CPPFLAGS) -DREPOS_PLUGINDIR=\"$(reposplugindir)\"
librepos_la_LIBADD = -lgatherutil
librepos_la_DEPENDENCIES = libgatherutil.la

librgather_la_SOURCES = rgather.c
librgather_la_LIBADD = -lgatherutil -lmcserv
librgather_la_DEPENDENCIES = libgatherutil.la libmcserv.la

libgather_la_SOURCES = mlist.c mretr.c mplugmgr.c mreg.c mreposp.c gather.c
libgather_la_CPPFLAGS = $(AM_CPPFLAGS) -DGATHER_PLUGINDIR=\"$(metricplugindir)\"
libgather_la_LIBADD = -lrrepos -lgatherutil
libgather_la_DEPENDENCIES = librrepos.la libgatherutil.la

# Plugin Utility 
libsysfswrapper_la_SOURCES = plugin/sysfswrapper.c plugin/sysfswrapper.h
libsysfswrapper_la_LIBADD = -lsysfs

libhypfs_la_SOURCES = plugin/hypfs.c plugin/hypfs.h

liblparutil_la_SOURCES = plugin/lparutil.c plugin/lparutil.h
liblparutil_la_LIBADD = -lhypfs
liblparutil_la_DEPENDENCIES = libhypfs.la

libchannelutil_la_SOURCES = plugin/channelutil.c plugin/channelutil.h
libchannelutil_la_LIBADD = -lsysfs -lsysfswrapper
libchannelutil_la_DEPENDENCIES = libsysfswrapper.la

# Gather daemons
sbin_PROGRAMS = gatherd reposd

gatherd_SOURCES = gatherd.c
gatherd_LDADD = -lgather -lmcserv
gatherd_DEPENDENCIES = libgather.la libmcserv.la

reposd_SOURCES = reposd.c sforward.c
reposd_LDADD = -lrepos -lmcserv -lrcserv
reposd_DEPENDENCIES = librepos.la libmcserv.la librcserv.la

# Gather CLIs
bin_PROGRAMS = gatherctl reposctl reposdump

gatherctl_SOURCES = gatherctl.c
gatherctl_LDADD = -lrgather
gatherctl_DEPENDENCIES = librgather.la

reposctl_SOURCES = reposctl.c
reposctl_LDADD = -lrrepos
reposctl_DEPENDENCIES = librrepos.la

reposdump_SOURCES = reposdump.c
reposdump_CPPFLAGS = $(AM_CPPFLAGS) -DREPOS_PLUGINDIR=\"$(reposplugindir)\"
reposdump_LDADD = -lgatherutil -lrrepos
reposdump_DEPENDENCIES = libgatherutil.la librrepos.la

# Gatherer Plugins
metricplugin_LTLIBRARIES = libmetricOperatingSystem.la \
	libmetricLocalFileSystem.la \
	libmetricUnixProcess.la \
	libmetricProcessor.la \
	libmetricNetworkPort.la \
	libmetricIPProtocolEndpoint.la \
	libmetricXen.la \
	libmetricKvm.la \
	libmetriczECKD.la \
	libmetriczLPAR.la \
	libmetriczCEC.la \
	libmetriczCH.la

libmetricOperatingSystem_la_SOURCES=plugin/metricOperatingSystem.c
libmetricOperatingSystem_la_LIBADD=-lgatherutil
libmetricOperatingSystem_la_DEPENDENCIES=libgatherutil.la

libmetricLocalFileSystem_la_SOURCES=plugin/metricLocalFileSystem.c
libmetricLocalFileSystem_la_LIBADD=-lgatherutil
libmetricLocalFileSystem_la_DEPENDENCIES=libgatherutil.la

libmetricUnixProcess_la_SOURCES=plugin/metricUnixProcess.c

libmetricProcessor_la_SOURCES=plugin/metricProcessor.c
libmetricProcessor_la_LIBADD=-lgatherutil
libmetricProcessor_la_DEPENDENCIES=libgatherutil.la

libmetricXen_la_SOURCES=plugin/metricXenV2.c plugin/metricVirt.c
libmetricXen_la_LIBADD=-lgatherutil -lvirt
libmetricXen_la_DEPENDENCIES=libgatherutil.la

libmetricKvm_la_SOURCES=plugin/metricKvm.c plugin/metricVirt.c
libmetricKvm_la_LIBADD=-lgatherutil -lvirt
libmetricKvm_la_DEPENDENCIES=libgatherutil.la

libmetricNetworkPort_la_SOURCES=plugin/metricNetworkPort.c

libmetricIPProtocolEndpoint_la_SOURCES=plugin/metricIPProtocolEndpoint.c

libmetriczECKD_la_SOURCES=plugin/metriczECKD.c
libmetriczECKD_la_LIBADD=-lsysfswrapper -lgatherutil
libmetriczECKD_la_DEPENDENCIES=libsysfswrapper.la

libmetriczLPAR_la_SOURCES=plugin/metriczLPAR.c
libmetriczLPAR_la_LIBADD=-llparutil -lgatherutil
libmetriczLPAR_la_DEPENDENCIES=liblparutil.la

libmetriczCEC_la_SOURCES=plugin/metriczCEC.c
libmetriczCEC_la_LIBADD=-llparutil -lgatherutil
libmetriczCEC_la_DEPENDENCIES=liblparutil.la

libmetriczCH_la_SOURCES=plugin/metriczCH.c
libmetriczCH_la_LIBADD=-lchannelutil -lgatherutil
libmetriczCH_la_DEPENDENCIES=libchannelutil.la

reposplugin_LTLIBRARIES = librepositoryOperatingSystem.la \
	librepositoryLocalFileSystem.la \
	librepositoryUnixProcess.la \
	librepositoryProcessor.la \
	librepositoryNetworkPort.la \
	librepositoryIPProtocolEndpoint.la \
	librepositoryXen.la \
	librepositoryKvm.la \
	librepositoryzECKD.la \
	librepositoryzLPAR.la \
	librepositoryzCEC.la \
	librepositoryzCH.la

librepositoryOperatingSystem_la_SOURCES=plugin/repositoryOperatingSystem.c
librepositoryOperatingSystem_la_LIBADD=-lgatherutil
librepositoryOperatingSystem_la_DEPENDENCIES=libgatherutil.la

librepositoryLocalFileSystem_la_SOURCES=plugin/repositoryLocalFileSystem.c
librepositoryLocalFileSystem_la_LIBADD=-lgatherutil
librepositoryLocalFileSystem_la_DEPENDENCIES=libgatherutil.la

librepositoryUnixProcess_la_SOURCES=plugin/repositoryUnixProcess.c

librepositoryProcessor_la_SOURCES=plugin/repositoryProcessor.c
librepositoryProcessor_la_LIBADD=-lgatherutil
librepositoryProcessor_la_DEPENDENCIES=libgatherutil.la

librepositoryXen_la_SOURCES=plugin/repositoryXen.c
librepositoryXen_la_LIBADD=-lgatherutil
librepositoryXen_la_DEPENDENCIES=libgatherutil.la

librepositoryKvm_la_SOURCES=plugin/repositoryKvm.c
librepositoryKvm_la_LIBADD=-lgatherutil
librepositoryKvm_la_DEPENDENCIES=libgatherutil.la

librepositoryNetworkPort_la_SOURCES=plugin/repositoryNetworkPort.c

librepositoryIPProtocolEndpoint_la_SOURCES=plugin/repositoryIPProtocolEndpoint.c
librepositoryzECKD_la_SOURCES=plugin/repositoryzECKD.c
librepositoryzECKD_la_LIBADD=-lgatherutil

librepositoryzLPAR_la_SOURCES=plugin/repositoryzLPAR.c
librepositoryzLPAR_la_LIBADD=-lgatherutil

librepositoryzCEC_la_SOURCES=plugin/repositoryzCEC.c
librepositoryzCEC_la_LIBADD=-lgatherutil

librepositoryzCH_la_SOURCES=plugin/repositoryzCH.c
librepositoryzCH_la_LIBADD=-lchannelutil -lgatherutil -lm
librepositoryzCH_la_DEPENDENCIES=libchannelutil.la

# Gather utility test programs (not installed)
noinst_PROGRAMS = mcfgtest mtracetest merrnotest mcstest mcctest \
	rcstest rcctest \
        hyptest channelclt

mcfgtest_SOURCES = util/mcfgtest.c
mcfgtest_LDADD = -lgatherutil
mcfgtest_DEPENDENCIES = libgatherutil.la

mtracetest_SOURCES = util/mtracetest.c
mtracetest_LDADD = -lgatherutil
mtracetest_DEPENDENCIES = libgatherutil.la

merrnotest_SOURCES = util/merrnotest.c
merrnotest_LDADD = -lgatherutil
merrnotest_DEPENDENCIES = libgatherutil.la

mcstest_SOURCES = comms/mcstest.c
mcstest_LDADD = -lmcserv
mcstest_DEPENDENCIES = libmcserv.la

mcctest_SOURCES = comms/mcctest.c
mcctest_LDADD = -lmcserv
mcctest_DEPENDENCIES = libmcserv.la

rcstest_SOURCES = comms/rcstest.c
rcstest_LDADD = -lrcserv
rcstest_DEPENDENCIES = librcserv.la

rcctest_SOURCES = comms/rcctest.c
rcctest_LDADD = -lrcserv
rcctest_DEPENDENCIES = librcserv.la

EXTRA_DIST += util/test.cfg

# Plugin Test

hyptest_SOURCES = plugin/hyptest.c
hyptest_LDADD = -lhypfs
hyptest_DEPENDENCIES = libhypfs.la

channelclt_SOURCES = plugin/channelclt.c 
channelclt_LDADD = -lsysfs -lchannelutil -lsysfswrapper
channelclt_DEPENDENCIES = libsysfswrapper.la libchannelutil.la

# Gather config files
sysconf_DATA = gatherd.conf reposd.conf
EXTRA_DIST += $(sysconf_DATA)

# We must explicity add the RPM spec file to the distribution package
EXTRA_DIST+=$(PACKAGE).spec $(PACKAGE).rh.spec

EXTRA_DIST += samples

install-data-hook:
	$(mkdir_p) $(DESTDIR)$(gatherrundir)

postinstall:
	$(MAKE) -C $(PROVIDER_SUBDIR) postinstall

preuninstall:
	$(MAKE) -C $(PROVIDER_SUBDIR) preuninstall

dist-hook:
	test -d "$(distdir)" &&	rm -rf `find $(distdir) -type d -name CVS`
