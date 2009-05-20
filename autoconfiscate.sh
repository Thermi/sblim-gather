#!/bin/sh
# $Id: autoconfiscate.sh,v 1.3 2009/05/20 19:39:55 tyreld Exp $
# ============================================================================
# (C) Copyright IBM Corp. 2005, 2009
#
# THIS FILE IS PROVIDED UNDER THE TERMS OF THE ECLIPSE PUBLIC LICENSE
# ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
# CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
#
# You can obtain a current copy of the Eclipse Public License from
# http://www.eclipse.org/legal/epl-v10.html
#
# Author:       Viktor Mihajlovski, <mihajlov@de.ibm.com>
# Contributors: Dr. Gareth S. Bestor, <bestorga@us.ibm.com>
# Last Updated: April 15, 2005
# Description:
#    Setup autoconf/automake build environment for package.
#    Run this script as the first step of building this package.
# ============================================================================
# NO CHANGES SHOULD BE NECESSARY TO THIS FILE
# ============================================================================

echo "Running aclocal ..." &&
aclocal --force &&

echo "Running autoheader ..." &&
autoheader --force &&

echo "Running libtool ..." &&
libtoolize --force && 

echo "Running automake ..." &&
automake --add-missing --force-missing &&

echo "Running autoconf ..." &&
autoconf --force &&

echo "You may now run ./configure"

