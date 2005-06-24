#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(sblim-cmpi-gather, 2.0.0, sblim-devel@lists.sourceforge.net)
AC_CONFIG_SRCDIR([OSBase_MetricDefForMEProvider.c])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE

# Features
AC_ARG_ENABLE(debug,
	[AS_HELP_STRING(--enable-debug,enable debug output.)])

# CIM Server related options
AC_ARG_VAR([TESTSUITEDIR],[the directory where the SBLIM testsuite resides.])
AC_ARG_VAR([PROVIDERDIR],[the directory where the CMPI providers will be installed.])
AC_ARG_VAR([CIMSERVER],[the target CIM server (pegasus|sfcb|openwbem|sniacimom).])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memmove strcasecmp strchr strdup strstr])

# CPPFLAGS for Provider Compilation
CPPFLAGS="-DCMPI_VERSION=90 $CPPFLAGS"

# Check for required CMPI header files (defined in acinclude.m4)
CHECK_CMPI

# Check for required libraries (defined in acinclude.m4)
CHECK_PROVIDERDIR

# Check for CIM Server (defined in acinclude.m4)
CHECK_CIMSERVER

# Check for SBLIM Include (defined in acinclude.m4)
CHECK_SBLIM_BASE

# Check for SBLIM Test Suite (defined in acinclude.m4)
CHECK_TESTSUITE

AC_CONFIG_FILES([Makefile])

if test "$enable_debug" == "yes"
then
	CPPFLAGS="-DDEBUG $CPPFLAGS"
else
	enable_debug=no
fi

# Display configuration options
echo "-------------------------------------------------------"
echo "Configuration for $PACKAGE (provider) complete."
echo ""
echo "CIMSERVER: " $CIMSERVER
echo "PROVIDERDIR: " $PROVIDERDIR
echo "Test Suite Support" $TESTSUITE_SUPPORT
if test x"$TESTSUITEDIR" != x; then
echo "TESTSUITEDIR " $TESTSUITEDIR
fi
echo "CPPFLAGS:" $CPPFLAGS
echo "-------------------------------------------------------"

AC_OUTPUT
