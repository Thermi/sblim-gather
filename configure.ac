#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([sblim-gather],[2.0.2],[sblim-devel@lists.sourceforge.net])
AC_CONFIG_SRCDIR([cimplug.h])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE

# Host Type
AC_CANONICAL_HOST
case $host_cpu in
     s390*) ARCHDEF="-D S390";;
     **)    ARCHDEF="-D GENERIC";;
esac

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

# Features
AC_ARG_ENABLE(provider,
	[AS_HELP_STRING(--disable-provider,disable building the provider.)],
	[enable_provider=$enableval],
	[enable_provider="yes"]
	)
AC_ARG_ENABLE(debug,
	[AS_HELP_STRING(--enable-debug,enable debug output.)])

# DECLARE ANY SPECIAL CUSTOM CONFIGURE COMMAND LINE OPTIONS HERE
AC_ARG_VAR([TESTSUITEDIR],[the directory where the SBLIM testsuite resides.])
AC_ARG_VAR([PROVIDERDIR],[the directory where the CMPI providers will be installed.])
AC_ARG_VAR([CIMSERVER],[the target CIM server (pegasus|sfcb|openwbem|sniacimom).])

# Checks for libraries.
AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([dl], [dlopen])

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h malloc.h memory.h mntent.h netdb.h netinet/in.h stddef.h stdlib.h string.h sys/file.h sys/socket.h sys/time.h sys/vfs.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_FORK
AC_FUNC_GETMNTENT
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([atexit dup2 gethostbyname gethostname getmntent gettimeofday inet_ntoa memmove memset socket strcasecmp strchr strdup strerror strrchr strspn strstr uname])

if test $enable_provider == "yes"
then
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

PROVIDER_SUBDIR=provider
AC_SUBST(PROVIDER_SUBDIR)
AC_CONFIG_FILES([provider/Makefile])
fi

# Check for SBLIM Test Suite (defined in acinclude.m4)
CHECK_TESTSUITE

AC_CONFIG_FILES([Makefile sblim-gather.spec])

AC_SUBST(ARCHDEF)

AM_CONDITIONAL(PROVIDER,[test "$enable_provider" == "yes"]) 

if test "$enable_debug" == "yes"
then
	CPPFLAGS="-DDEBUG $CPPFLAGS"
else
	enable_debug=no
	CPPFLAGS="-DNOTRACE $CPPFLAGS"
fi

# Display configuration options
echo "-------------------------------------------------------"
echo "Configuration for $PACKAGE complete."
echo ""
echo "The following configuration options have been selected:"
echo "Enable Debug: " $enable_debug
echo "Enable Provider: " $enable_provider
if test $enable_provider == yes
then
echo "CIMSERVER: " $CIMSERVER
echo "PROVIDERDIR: " $PROVIDERDIR
if test x"$TESTSUITEDIR" == x; then
   TESTSUITE_SUPPORT=Disabled
else	
   TESTSUITE_SUPPORT=Enabled
fi
echo "Test Suite Support" $TESTSUITE_SUPPORT
if test x"$TESTSUITEDIR" != x; then
echo "TESTSUITEDIR " $TESTSUITEDIR
fi
fi
echo "CPPFLAGS:" $CPPFLAGS
echo "-------------------------------------------------------"

AC_OUTPUT

echo "You may now run make"
