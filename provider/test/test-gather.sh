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
    
USERID=
PASSWORD=

while [ "$#" -gt 0 ]
do
  COMMAND=$1
  shift
  if [[ -n "$COMMAND" && "$COMMAND" == "-u" ]]; then
      if [[ -n "$1" ]]; then 
	  USERID=$1;
      else
	  echo "test-gather.sh : Please specify a UserID after -u"; 
	  exit 1;
      fi
  elif [[ -n "$COMMAND" && "$COMMAND" == "-p" ]]; then
      if [[ -z "$1" ]]; then 
	  echo "test-gather.sh : Please specify a password for UserID $USERID after -p"; exit 1;
      else 
	  PASSWORD=$1;
      fi
  fi
done

if [[ -n $USERID && -z $PASSWORD ]]; then
    echo "test-gather.sh : Please specify a password for UserID $USERID : option -p"; 
    exit 1;
elif  [[ -n $USERID && -n $PASSWORD ]]; then
    export SBLIM_TESTSUITE_ACCESS="$USERID:$PASSWORD@";
fi

# Initialize Gatherer Service 
#echo k | gatherctl
#echo d | gatherctl
GATHER=`wbemein http://${SBLIM_TESTSUITE_ACCESS}localhost/root/cimv2:Linux_MetricGatherer 2>&1`
if echo $GATHER | grep CIM_ERR > /dev/null
then
    echo "Error occurred listing the Metric Gatherer ... is the CIMOM running?"
    echo $GATHER
    exit -1
fi

# Stopping everything to be in a defined state then start
wbemcm http://${SBLIM_TESTSUITE_ACCESS}$GATHER StopSampling > /dev/null
wbemcm http://${SBLIM_TESTSUITE_ACCESS}$GATHER StopService > /dev/null

wbemcm http://${SBLIM_TESTSUITE_ACCESS}$GATHER StartService > /dev/null
SAMPLING=`wbemcm http://${SBLIM_TESTSUITE_ACCESS}$GATHER StartSampling`
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

OPTS=
if [ -n $USERID ]
then
OPTS="$OPTS -u $USERID"
fi
if [ -n $PASSWORD ]
then
OPTS="$OPTS -p $PASSWORD"
fi
while(($i<=$max))
do
  . run.sh ${CLASSNAMES[$i]} $OPTS || exit 1;
  i=$i+1;
done
