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
SBLIM_TESTSUITE_ACCESS=

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
    SBLIM_TESTSUITE_ACCESS="$USERID:$PASSWORD@";
fi

CIMTEST=`wbemgc http://${SBLIM_TESTSUITE_ACCESS}localhost/root/cimv2:CIM_ManagedElement 2>&1`
if ! echo $CIMTEST | grep anaged > /dev/null
then
    echo "Error occurred ... is the CIMOM running?"
    echo $CIMTEST
    exit -1
fi

# Initialize Gatherer Service 
#echo k | gatherctl
#echo d | gatherctl
GATHER=`wbemein http://${SBLIM_TESTSUITE_ACCESS}localhost/root/cimv2:Linux_MetricGatherer 2>&1`
if echo $GATHER | grep -v Linux_MetricGatherer > /dev/null
then
    echo "Error occurred listing the Metric Gatherer ... are the providers installed?"
    echo $GATHER
    exit -1
fi

# Stopping everything to be in a defined state then start
wbemcm http://${SBLIM_TESTSUITE_ACCESS}$GATHER StopSampling > /dev/null
wbemcm http://${SBLIM_TESTSUITE_ACCESS}$GATHER StopService > /dev/null

wbemcm http://${SBLIM_TESTSUITE_ACCESS}$GATHER StartService > /dev/null
SAMPLING=`wbemcm http://${SBLIM_TESTSUITE_ACCESS}$GATHER StartSampling`
if ! echo $SAMPLING | grep -i TRUE > /dev/null
then
    echo "The Metric Gather is not sampling - have to quit"
    exit -1
fi

# Initialize Repository Service 
#echo k | reposctl
#echo d | reposctl
REPOS=`wbemein http://${SBLIM_TESTSUITE_ACCESS}localhost/root/cimv2:Linux_MetricRepositoryService 2>&1`
if echo $REPOS | grep -v Linux_MetricRepositoryService > /dev/null
then
    echo "Error occurred listing the Repository Service ... are the providers installed?"
    echo $REPOS
    exit -1
fi

# Stopping everything to be in a defined state then start
wbemcm http://${SBLIM_TESTSUITE_ACCESS}$REPOS StopService > /dev/null

wbemcm http://${SBLIM_TESTSUITE_ACCESS}$REPOS StartService > /dev/null

# Wait 60 seconds to make sure that enough samples are generated
echo "need to wait 60 seconds for gatherd and sampling initialization ... please stand by ... we will be back ;-) ...";
sleep 60
echo "... back ...";

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
    [12]=Linux_IPProtocolEndpointMetric \
    [11]=Linux_IPProtocolEndpointMetricValue \
    )

declare -i max=12;
declare -i i=0;

OPTS=
if [ -n "$USERID" ]
then
OPTS="$OPTS -u $USERID"
fi
if [ -n "$PASSWORD" ]
then
OPTS="$OPTS -p $PASSWORD"
fi

while(($i<=$max))
do
  . run.sh ${CLASSNAMES[$i]} $OPTS || exit 1;
  i=$i+1;
done
