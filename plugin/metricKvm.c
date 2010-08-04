/*
 * $Id: metricKvm.c,v 1.4 2010/08/04 23:24:36 tyreld Exp $
 *
 * (C) Copyright IBM Corp. 2009
 *
 * THIS FILE IS PROVIDED UNDER THE TERMS OF THE ECLIPSE PUBLIC LICENSE
 * ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
 * CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
 *
 * You can obtain a current copy of the Eclipse Public License from
 * http://www.opensource.org/licenses/eclipse-1.0.php
 *
 * Author:       Tyrel Datwyler	<tyreld@us.ibm.com>
 * Contributors: 
 *
 * Description:
 * Metrics gatherer Plugin of the following KVM specific metrics :
 * 
 *    ActiveVirtualProcessors
 *    HostFreePhysicalMemory
 *    VirtualSystemState
 * 
 * plus the following metrics which are only intended for internal usage:
 *    _Internal_CPUTime
 *    _Internal_TotalCPUTime
 *    _Internal_Memory
 *    _Internal10m_CPUTime
 *    _Internal10m_TotalCPUTime
 *
*/


// #define DEBUG

#include "metricVirt.h"

#include <mplugin.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>

/* --- metric callback retrievers to be exported --- */
static MetricDefinition metricDef[8];

/* --- required plugin functions --- */
int _DefinedMetrics(MetricRegisterId * mr,
		    const char *pluginname,
		    size_t * mdnum, MetricDefinition ** md)
{
	int conn;

#ifdef DEBUG
    fprintf(stderr, "retrieving metric definitions\n");
#endif
	
	conn = connectHypervisor(KVM_HYP);
	
#ifdef DEBUG
    fprintf(stderr, "--- %s(%i) : Xen is %s\n",
	    __FILE__, __LINE__,
	    (conn) ? "active" : "NOT active");
#endif


    if (mr == NULL || mdnum == NULL || md == NULL) {
#ifdef DEBUG
	fprintf(stderr,
		"--- %s(%i) : invalid parameter list\n",
		__FILE__, __LINE__);
#endif
	return -1;
    }

    if (conn) {
	metricDef[0].mdVersion = MD_VERSION;
	metricDef[0].mdName = "_Internal_CPUTime";
	metricDef[0].mdReposPluginName = "librepositoryKvm.so";
	metricDef[0].mdId = mr(pluginname, metricDef[0].mdName);
	metricDef[0].mdSampleInterval = 60;
	metricDef[0].mproc = virtMetricRetrCPUTime;
	metricDef[0].mdeal = free;

	metricDef[1].mdVersion = MD_VERSION;
	metricDef[1].mdName = "_Internal_TotalCPUTime";
	metricDef[1].mdReposPluginName = "librepositoryKvm.so";
	metricDef[1].mdId = mr(pluginname, metricDef[1].mdName);
	metricDef[1].mdSampleInterval = 60;
	metricDef[1].mproc = virtMetricRetrTotalCPUTime;
	metricDef[1].mdeal = free;

	metricDef[2].mdVersion = MD_VERSION;
	metricDef[2].mdName = "ActiveVirtualProcessors";
	metricDef[2].mdReposPluginName = "librepositoryKvm.so";
	metricDef[2].mdId = mr(pluginname, metricDef[2].mdName);
	metricDef[2].mdSampleInterval = 60;
	metricDef[2].mproc = virtMetricRetrActiveVirtualProcessors;
	metricDef[2].mdeal = free;

	metricDef[3].mdVersion = MD_VERSION;
	metricDef[3].mdName = "_Internal_Memory";
	metricDef[3].mdReposPluginName = "librepositoryKvm.so";
	metricDef[3].mdId = mr(pluginname, metricDef[3].mdName);
	metricDef[3].mdSampleInterval = 60;
	metricDef[3].mproc = virtMetricRetrInternalMemory;
	metricDef[3].mdeal = free;

	metricDef[4].mdVersion = MD_VERSION;
	metricDef[4].mdName = "HostFreePhysicalMemory";
	metricDef[4].mdReposPluginName = "librepositoryKvm.so";
	metricDef[4].mdId = mr(pluginname, metricDef[4].mdName);
	metricDef[4].mdSampleInterval = 60;
	metricDef[4].mproc = virtMetricRetrHostFreePhysicalMemory;
	metricDef[4].mdeal = free;

	metricDef[5].mdVersion = MD_VERSION;
	metricDef[5].mdName = "_Internal10m_CPUTime";
	metricDef[5].mdReposPluginName = "librepositoryKvm.so";
	metricDef[5].mdId = mr(pluginname, metricDef[5].mdName);
	metricDef[5].mdSampleInterval = 600;
	metricDef[5].mproc = virtMetricRetrCPUTime;
	metricDef[5].mdeal = free;

	metricDef[6].mdVersion = MD_VERSION;
	metricDef[6].mdName = "_Internal10m_TotalCPUTime";
	metricDef[6].mdReposPluginName = "librepositoryKvm.so";
	metricDef[6].mdId = mr(pluginname, metricDef[6].mdName);
	metricDef[6].mdSampleInterval = 600;
	metricDef[6].mproc = virtMetricRetrTotalCPUTime;
	metricDef[6].mdeal = free;

	metricDef[7].mdVersion = MD_VERSION;
	metricDef[7].mdName = "VirtualSystemState";
	metricDef[7].mdReposPluginName = "librepositoryKvm.so";
	metricDef[7].mdId = mr(pluginname, metricDef[7].mdName);
	metricDef[7].mdSampleInterval = 60;
	metricDef[7].mproc = virtMetricRetrVirtualSystemState;
	metricDef[7].mdeal = free;

	*mdnum = 8;
    } else {
	*mdnum = 0;
    }

    *md = metricDef;
    return 0;
}

int _StartStopMetrics(int starting)
{

    int i;
#ifdef DEBUG
    fprintf(stderr, "--- %s(%i) : %s metric processing\n",
	    __FILE__, __LINE__, starting ? "Starting" : "Stopping");
#endif

    // initialize string array with zero, so strings can be free'd
    // for (i = 0; i < 2; i++) {
	memset((char *) &domain_statistics.domain_name[0],
	       0, MAX_DOMAINS * sizeof(char *));
    // }
    return 0;
}
