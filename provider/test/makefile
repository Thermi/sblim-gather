# Makefile for testing the SBLIM Base Instrumentation using the 
# SBLIM Test Suite package

# change the following variables to fit your system

TESTSUITE=$(HOME)/src/checkout/testsuite
TESTSUITECIM=$(TESTSUITE)/cim
TESTSUITESYSTEM=$(TESTSUITE)/system/linux


# in most cases it is not necessary to change the following stuff

all: 	test

install: all
	install test-gather.sh $(TESTSUITE)
	install cim/*.cim $(TESTSUITECIM)
	install system/linux/*.system $(TESTSUITESYSTEM)
	install system/linux/*.sh $(TESTSUITESYSTEM)
#	install system/linux/*.pl $(TESTSUITESYSTEM)

test:
	@[ -d $(TESTSUITE) ] || ( echo $(TESTSUITE) does not exist && false)
	@[ -d $(TESTSUITECIM) ] || ( echo $(TESTSUITECIM) does not exist && false)
	@[ -d $(TESTSUITESYSTEM) ] || ( echo $(TESTSUITESYSTEM) does not exist && false)

uninstall: 
	$(RM) $(TESTSUITE)/test-gather.sh \
	$(RM) $(TESTSUITECIM)/Linux_MetricGatherer.cim \
	$(RM) $(TESTSUITECIM)/Linux_NetworkPortMetric.cim \
	$(RM) $(TESTSUITECIM)/Linux_LocalFileSystemMetric.cim \
	$(RM) $(TESTSUITECIM)/Linux_ProcessorMetric.cim \
	$(RM) $(TESTSUITECIM)/Linux_UnixProcessMetric.cim \
	$(RM) $(TESTSUITECIM)/Linux_OperatingSystemMetric.cim \
	$(RM) $(TESTSUITECIM)/Linux_NetworkPortMetricValue.cim \
	$(RM) $(TESTSUITECIM)/Linux_LocalFileSystemMetricValue.cim \
	$(RM) $(TESTSUITECIM)/Linux_ProcessorMetricValue.cim \
	$(RM) $(TESTSUITECIM)/Linux_UnixProcessMetricValue.cim \
	$(RM) $(TESTSUITECIM)/Linux_OperatingSystemMetricValue.cim \
	$(RM) $(TESTSUITESYSTEM)/Linux_MetricGatherer.system \
	$(RM) $(TESTSUITESYSTEM)/Linux_NetworkPortMetric.system \
	$(RM) $(TESTSUITESYSTEM)/Linux_LocalFileSystemMetric.system \
	$(RM) $(TESTSUITESYSTEM)/Linux_ProcessorMetric.system \
	$(RM) $(TESTSUITESYSTEM)/Linux_UnixProcessMetric.system \
	$(RM) $(TESTSUITESYSTEM)/Linux_OperatingSystemMetric.system \
	$(RM) $(TESTSUITESYSTEM)/Linux_NetworkPortMetricValue.system \
	$(RM) $(TESTSUITESYSTEM)/Linux_LocalFileSystemMetricValue.system \
	$(RM) $(TESTSUITESYSTEM)/Linux_ProcessorMetricValue.system \
	$(RM) $(TESTSUITESYSTEM)/Linux_UnixProcessMetricValue.system \
	$(RM) $(TESTSUITESYSTEM)/Linux_OperatingSystemMetricValue.system \
	$(RM) $(TESTSUITESYSTEM)/createKeyFiles.sh


