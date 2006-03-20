#
# $Id: sblim-gather.rh.spec,v 1.1 2006/03/20 17:15:12 mihajlov Exp $
#
# Package spec for sblim-gather
#

%define peg24 %{?rhel4:1}%{!?rhel4:0}

BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Summary: SBLIM Performance Data Gatherer
Name: sblim-gather
Version: 2.0.99k
Release: 1.rh%{?rhel4:el4}
Group: Systems Management/Base
URL: http://www.sblim.org
License: CPL

Source0: http://prdownloads.sourceforge.net/sblim/%{name}-%{version}.tar.bz2

%if %{peg24}
%define providerdir %{_libdir}/Pegasus/providers
BuildRequires: sblim-cmpi-devel
Requires: tog-pegasus >= 2.4
%else
%define providerdir %{_libdir}/cmpi
BuildRequires: tog-pegasus-devel >= 2.5
Requires: tog-pegasus >= 2.5
%endif

BuildRequires: sblim-cmpi-base-devel
Requires: sblim-cmpi-base

%Description
Standards Based Linux Instrumentation Performance Data Gatherer and Providers

%Package devel
Summary: SBLIM Gatherer Development Support
Group: Systems Management/Base
Requires: %{name} = %{version}

%Description devel
This package is needed to develop new plugins for the gatherer.

%Package test
Summary: SBLIM Gatherer Testcase Files
Group: Systems Management/Base
Requires: %{name} = %{version}
Requires: sblim-wbemcli

%Description test
SBLIM Gatherer Testcase Files for the SBLIM Testsuite

%prep

%setup -q

%build

%configure TESTSUITEDIR=%{_datadir}/sblim-testsuite \
	CIMSERVER=pegasus PROVIDERDIR=%{providerdir}

make %{?_smp_mflags}

%install

rm -rf $RPM_BUILD_ROOT

make DESTDIR=$RPM_BUILD_ROOT install

# remove unused libtool files
rm -f $RPM_BUILD_ROOT/%{_libdir}/*a
rm -f $RPM_BUILD_ROOT/%{providerdir}/*a
rm -f $RPM_BUILD_ROOT/%{_libdir}/gather/*plug/*a

# remove non-devel .so's from libdir
find $RPM_BUILD_ROOT/%{_libdir} -maxdepth 1 -name "*.so" ! -name libgatherutil.so \
	-exec rm {} \;

%pre

%define SCHEMA 	%{_datadir}/%{name}/Linux_Metric.mof %{_datadir}/%{name}/Linux_IPProtocolEndpointMetric.mof %{_datadir}/%{name}/Linux_LocalFileSystemMetric.mof %{_datadir}/%{name}/Linux_NetworkPortMetric.mof %{_datadir}/%{name}/Linux_OperatingSystemMetric.mof %{_datadir}/%{name}/Linux_ProcessorMetric.mof %{_datadir}/%{name}/Linux_UnixProcessMetric.mof %{_datadir}/%{name}/Linux_XenMetric.mof 
%define REGISTRATION %{_datadir}/%{name}/Linux_IPProtocolEndpointMetric.registration %{_datadir}/%{name}/Linux_LocalFileSystemMetric.registration %{_datadir}/%{name}/Linux_Metric.registration %{_datadir}/%{name}/Linux_NetworkPortMetric.registration %{_datadir}/%{name}/Linux_OperatingSystemMetric.registration %{_datadir}/%{name}/Linux_ProcessorMetric.registration %{_datadir}/%{name}/Linux_UnixProcessMetric.registration %{_datadir}/%{name}/Linux_XenMetric.registration

# If upgrading, deregister old version
if [ $1 -gt 1 ]
then
  %{_datadir}/%{name}/provider-register.sh -t pegasus -d \
	-r %{REGISTRATION} -m %{SCHEMA} > /dev/null
fi

%post
# Register Schema and Provider - this is higly provider specific

%{_datadir}/%{name}/provider-register.sh -t pegasus \
	-r %{REGISTRATION} -m %{SCHEMA} > /dev/null

/sbin/ldconfig

%preun

# Deregister only if not upgrading 
if [ $1 -eq 0 ]
then
  %{_datadir}/%{name}/provider-register.sh -t pegasus -d \
	-r %{REGISTRATION} -m %{SCHEMA} > /dev/null
fi

%postun -p /sbin/ldconfig

%clean

rm -rf $RPM_BUILD_ROOT

%files

%defattr(-,root,root) 

%config(noreplace) %{_sysconfdir}/*.conf
%docdir %{_datadir}/doc/%{name}-%{version}
%{_bindir}/*
%{_sbindir}/*
%{_datadir}/%{name}
%{_datadir}/doc/%{name}-%{version}
%{_localstatedir}/run/gather
%{providerdir}
%{_libdir}/lib*.so.*
%{_libdir}/gather

%files devel
%{_libdir}/lib*.so
%{_includedir}/gather

%files test

%defattr(-,root,root)
%{_datadir}/sblim-testsuite

%changelog

* Fri Mar 17 2006 Viktor Mihajlovski <mihajlov@dyn-9-152-143-45.boeblingen.de.ibm.com> - 2.0.99k-1.rh%{?rhel4:el4}
- Cleanup in preparation for 2.1.0 for RH/Fedora

