#!/bin/sh

# test gather package

#******************************************************************************#

#export SBLIM_TESTSUITE_RUN=1;

#******************************************************************************#
# WBEMCLI available in version 1.4 ?
if ! wbemcli -v 2>&1 | grep 1.4 > /dev/null
then
    echo "wbemcli 1.4 must be installed"
    exit -1
fi
    
# Initialize Gatherer Service 
GATHER=`wbemein http://localhost/root/cimv2:Linux_MetricGatherer 2>&1`
if echo $GATHER | grep CIM_ERR > /dev/null
then
    echo "Error occurred listing the Metric Gatherer ... is the CIMOM running?"
    echo $GATHER
    exit -1
fi

# Stopping everything to be in a defined state then start
wbemcm http://$GATHER StopSampling > /dev/null
wbemcm http://$GATHER StopService > /dev/null

wbemcm http://$GATHER StartService > /dev/null
SAMPLING=`wbemcm http://$GATHER StartSampling`
if ! echo $SAMPLING | grep TRUE > /dev/null
then
    echo "The Metric Gather is not sampling - have to quit"
    exit -1
fi

declare -a CLASSNAMES[];
CLASSNAMES=([0]=Linux_MetricGatherer \
    [1]=Linux_NetworkPortMetric \
    [2]=Linux_LocalFileSystemMetric \
    [3]=Linux_ProcessorMetric \
    [4]=Linux_UnixProcessMetric \
    [5]=Linux_OperatingSystemMetric \
    [6]=Linux_NetworkPortMetricValue \
    [7]=Linux_LocalFileSystemMetricValue \
    [8]=Linux_ProcessorMetricValue \
    [9]=Linux_UnixProcessMetricValue \
    [10]=Linux_OperatingSystemMetricValue \
    )

declare -i max=10;
declare -i i=0;

while(($i<=$max))
do
  . run.sh ${CLASSNAMES[$i]} || exit 1;
  i=$i+1;
done
