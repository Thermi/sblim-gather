#!/bin/sh

HW=`uname -m`;

echo $HW | grep "i386" >/dev/null && echo "INTEL";
echo $HW | grep "i486" >/dev/null && echo "INTEL";
echo $HW | grep "i586" >/dev/null && echo "INTEL";
echo $HW | grep "i686" >/dev/null && echo "INTEL";

echo $HW | grep "s390" >/dev/null && echo "S390";

echo $HW | grep "ppc" >/dev/null && echo "PPC";
