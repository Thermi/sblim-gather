#!/bin/sh


#***********************************************************************#
#                          MetricGatherer                               #
#-----------------------------------------------------------------------#

# SystemCreationClassName

echo 'Linux_ComputerSystem' > MetricGatherer.keys

# SystemName

echo `hostname` | grep `dnsdomainname` >/dev/null\
&& echo `hostname` >> MetricGatherer.keys \
|| echo `hostname`'.'`dnsdomainname` >> MetricGatherer.keys


# CreationClassName

echo 'Linux_MetricGatherer' >> MetricGatherer.keys

# Name

echo 'gatherd' >> MetricGatherer.keys
