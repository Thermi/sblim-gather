#
# sblim-gather-1.2.4.spec
#
# Package spec for CMPI Base
#

BuildRoot: /var/tmp/buildroot

Summary: SBLIM Performance Data Gatherer and Providers
Name: sblim-gather
Version: 1.0.1
Release: 2
Group: Systems Management/Base
Copyright: Common Public Licence http://oss.software.ibm.com/developerworks/opensource/license-cpl.html
Packager: Viktor Mihajlovski <mihajlov@de.ibm.com>

BuildRequires: cmpi-devel
BuildRequires: sblim-cmpi-base-devel >= 1.2.3

Requires: sblim-cmpi-base >= 1.2.3

Source0: http://www-126-ibm.com/pub/sblim/sblim-gather/%{name}-%{version}-%{release}.tar.gz

%Description
Standards Based Linux Instrumentation Performance Data Gatherer and Providers
for OSBase Metrics.

%prep

%setup -n %{name}-%{version}-%{release}

export PATCH_GET=0

#%patch0 -p0

%build

make
make -C provider  PEGASUS= STANDALONE=1

%clean

if [ `id -ur` != 0 ]
then
# paranoia check 
	rm -rf $RPM_BUILD_ROOT 
fi

%install

mkdir -p $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/lib/cmpi
mkdir -p $RPM_BUILD_ROOT/usr/include/cmpi
mkdir -p $RPM_BUILD_ROOT/usr/share/cmpi/mof
mkdir -p $RPM_BUILD_ROOT/usr/share/cmpi/tests/cim
mkdir -p $RPM_BUILD_ROOT/usr/share/cmpi/tests/system/linux

make install INSTALLROOT=$RPM_BUILD_ROOT
make -C provider install INSTALL_ROOT=$RPM_BUILD_ROOT PEGASUS= STANDALONE=1
make -C provider/test install TESTSUITE=$RPM_BUILD_ROOT/usr/share/cmpi/tests

# SUSE specific
install -d $RPM_BUILD_ROOT/sbin/conf.d
install -m755 contrib/SuSEconfig.%{name} $RPM_BUILD_ROOT/sbin/conf.d

%files

%defattr(-,root,root) 
/usr/bin
/usr/lib
/usr/share/cmpi

# SUSE specific
/sbin/conf.d

# SUSE specific too
%post
echo "#run me once" > /var/adm/SuSEconfig/run-%{name} || true
