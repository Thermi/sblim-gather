# Makefile for SBLIM Base Instrumentation 


include ../../rules
include ../setting.cmpi

.PHONY: all install uninstall clean distclean

all:

install: 
	echo $(MOFMAKEFILE)
	$(MAKE) -f $(MOFMAKEFILE) install
ifeq ("$(MOFMAKEFILE)","makefile.pegasus")
	$(MAKE) -f makefile.OperatingSystem.pegasus install
	$(MAKE) -f makefile.UnixProcess.pegasus install
	$(MAKE) -f makefile.Processor.pegasus install
	$(MAKE) -f makefile.LocalFileSystem.pegasus install
	$(MAKE) -f makefile.NetworkPort.pegasus install
	$(MAKE) -f makefile.IPProtocolEndpoint.pegasus install
else
	$(MAKE) -f makefile.OperatingSystem.standalone install
	$(MAKE) -f makefile.UnixProcess.standalone install
	$(MAKE) -f makefile.Processor.standalone install
	$(MAKE) -f makefile.LocalFileSystem.standalone install
	$(MAKE) -f makefile.IPProtocolEndpoint.standalone install
endif

uninstall:

clean:

distclean:










