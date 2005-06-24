# $Id: Makefile.am,v 1.1 2005/06/24 12:04:56 mihajlov Exp $
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
# Description:  Automake input file for Gatherer Provider
# ==================================================================

# Start from an empty extra distribution file list
EXTRA_DIST=

# docdir defines where the documentation goes
#docdir=$(datadir)/doc/$(PACKAGE)-$(VERSION)

# plugin directories
cimplugindir=$(libdir)

#
# Automake instructions for documentation
#
#doc_DATA=

# ADD EXTRA DOC FILES IF PRESENT
#EXTRA_DIST+=$(doc_DATA)

#doc_DATA+=doc/rpmbuild.txt
#EXTRA_DIST+=doc

# providerdir defines where provider libraries will be installed (@PROVIDERDIR@ is set by the configure script). 
providerdir = @PROVIDERDIR@

# Generally useful flags
AM_CPPFLAGS = -I $(srcdir)/.. -I $(srcdir)/../util -I $(srcdir)/../comms
AM_CFLAGS = -Wall
AM_LDFLAGS = -L..

# Include files
noinst_HEADERS = OSBase_MetricUtil.h

# Gather CIM plugin libraries
cimplugin_LTLIBRARIES = libcimplugOperatingSystem.la \
	libcimplugLocalFileSystem.la \
	libcimplugUnixProcess.la \
	libcimplugProcessor.la \
	libcimplugNetworkPort.la \
	libcimplugIPProtocolEndpoint.la

libcimplugOperatingSystem_la_SOURCES=$(srcdir)/../plugin/cimplugOperatingSystem.c
libcimplugLocalFileSystem_la_SOURCES=$(srcdir)/../plugin/cimplugLocalFileSystem.c
libcimplugUnixProcess_la_SOURCES=$(srcdir)/../plugin/cimplugUnixProcess.c
libcimplugProcessor_la_SOURCES=$(srcdir)/../plugin/cimplugProcessor.c
libcimplugNetworkPort_la_SOURCES=$(srcdir)/../plugin/cimplugNetworkPort.c
libcimplugIPProtocolEndpoint_la_SOURCES=$(srcdir)/../plugin/cimplugIPProtocolEndpoint.c

# Gather CMPI provider utility libraries
lib_LTLIBRARIES = libOSBase_MetricUtil.la
libOSBase_MetricUtil_la_SOURCES=OSBase_MetricUtil.c
libOSBase_MetricUtil_la_LIBADD=-lrrepos

# Gather CMPI provider libraries
provider_LTLIBRARIES = libOSBase_MetricDefinitionProvider.la \
	libOSBase_MetricValueProvider.la \
	libOSBase_MetricGathererProvider.la \
	libOSBase_MetricRepositoryServiceProvider.la \
	libOSBase_MetricForMEProvider.la \
	libOSBase_MetricDefForMEProvider.la \
	libOSBase_MetricInstanceProvider.la \
	libOSBase_MetricIndicationProvider.la

libOSBase_MetricDefinitionProvider_la_SOURCES = OSBase_MetricDefinitionProvider.c
libOSBase_MetricDefinitionProvider_la_LIBADD = -lOSBase_MetricUtil

libOSBase_MetricValueProvider_la_SOURCES = OSBase_MetricValueProvider.c
libOSBase_MetricValueProvider_la_LIBADD = -lOSBase_MetricUtil

libOSBase_MetricGathererProvider_la_SOURCES = OSBase_MetricGathererProvider.c
libOSBase_MetricGathererProvider_la_LIBADD = -lOSBase_MetricUtil -lrgather @LIBSBLIMBASE@

libOSBase_MetricRepositoryServiceProvider_la_SOURCES = OSBase_MetricRepositoryServiceProvider.c
libOSBase_MetricRepositoryServiceProvider_la_LIBADD = -lOSBase_MetricUtil @LIBSBLIMBASE@

libOSBase_MetricForMEProvider_la_SOURCES = OSBase_MetricForMEProvider.c
libOSBase_MetricForMEProvider_la_LIBADD = -lOSBase_MetricUtil

libOSBase_MetricDefForMEProvider_la_SOURCES = OSBase_MetricDefForMEProvider.c
libOSBase_MetricDefForMEProvider_la_LIBADD = -lOSBase_MetricUtil

libOSBase_MetricInstanceProvider_la_SOURCES = OSBase_MetricInstanceProvider.c
libOSBase_MetricInstanceProvider_la_LIBADD = -lOSBase_MetricUtil

libOSBase_MetricIndicationProvider_la_SOURCES = OSBase_MetricIndicationProvider.c
libOSBase_MetricIndicationProvider_la_LIBADD = -lOSBase_MetricUtil

#
# Automake instructions for ./mof subdir
#
MOFS=	mof/Linux_Metric.mof \
        mof/Linux_IPProtocolEndpointMetric.mof \
	mof/Linux_LocalFileSystemMetric.mof \
	mof/Linux_NetworkPortMetric.mof \
	mof/Linux_OperatingSystemMetric.mof \
	mof/Linux_ProcessorMetric.mof \
	mof/Linux_UnixProcessMetric.mof 
REGS=mof/Linux_IPProtocolEndpointMetric.registration \
	mof/Linux_LocalFileSystemMetric.registration \
	mof/Linux_Metric.registration \
	mof/Linux_NetworkPortMetric.registration \
	mof/Linux_OperatingSystemMetric.registration \
	mof/Linux_ProcessorMetric.registration \
	mof/Linux_UnixProcessMetric.registration

# Register the provider(s) and class definition(s) to the current CIM server/CIMOM
postinstall:
	sh provider-register.sh -t @CIMSERVER@ -r $(REGS) -m $(MOFS)

preuninstall:
	sh provider-register.sh -d -t @CIMSERVER@ -r $(REGS) -m $(MOFS)

dist-hook:
	test -d "$(distdir)" &&	rm -rf `find $(distdir) -type d -name CVS`

# We must explicity add all the schema files to the distribution package
pkgdata_DATA=$(MOFS) $(REGS)
pkgdata_SCRIPTS=provider-register.sh start_gathering.sh stop_gathering.sh

EXTRA_DIST+=$(pkgdata_DATA) $(pkgdata_SCRIPTS)

#
# Automake instructions for ./test subdir
#
EXTRA_DIST+=test indicationtest
if TESTSUITE
testsuitedir=@TESTSUITEDIR@
testsuitesystemdir=@TESTSUITEDIR@/system/linux
testsuitecimdir=@TESTSUITEDIR@/cim

testsuite_SCRIPTS=test/test-gather.sh
testsuitesystem_DATA= test/system/linux/Linux_IPProtocolEndpointMetric.system \
	test/system/linux/Linux_IPProtocolEndpointMetricValue.system \
	test/system/linux/Linux_LocalFileSystemMetric.system \
	test/system/linux/Linux_LocalFileSystemMetricValue.system \
	test/system/linux/Linux_MetricGatherer.system \
	test/system/linux/Linux_NetworkPortMetric.system \
	test/system/linux/Linux_NetworkPortMetricValue.system \
	test/system/linux/Linux_OperatingSystemMetric.system \
	test/system/linux/Linux_OperatingSystemMetricValue.system \
	test/system/linux/Linux_ProcessorMetric.system \
	test/system/linux/Linux_ProcessorMetricValue.system \
	test/system/linux/Linux_UnixProcessMetric.system \
	test/system/linux/Linux_UnixProcessMetricValue.system

testsuitesystem_SCRIPTS=test/system/linux/gather-systemname.sh

testsuitecim_DATA= test/cim/Linux_IPProtocolEndpointMetric.cim \
	test/cim/Linux_IPProtocolEndpointMetricValue.cim \
	test/cim/Linux_LocalFileSystemMetric.cim \
	test/cim/Linux_LocalFileSystemMetricValue.cim \
	test/cim/Linux_MetricGatherer.cim \
	test/cim/Linux_NetworkPortMetric.cim \
	test/cim/Linux_NetworkPortMetricValue.cim \
	test/cim/Linux_OperatingSystemMetric.cim \
	test/cim/Linux_OperatingSystemMetricValue.cim \
	test/cim/Linux_ProcessorMetric.cim \
	test/cim/Linux_ProcessorMetricValue.cim \
	test/cim/Linux_UnixProcessMetric.cim \
	test/cim/Linux_UnixProcessMetricValue.cim 

runtest: install
	cd $(DESTDIR)$(TESTSUITEDIR) &&	./test-cmpi-base.sh

#doc_DATA+=README.TEST
endif