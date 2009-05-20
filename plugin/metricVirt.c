/*
 * $Id: metricVirt.c,v 1.4 2009/05/20 19:39:56 tyreld Exp $
 *
 * (C) Copyright IBM Corp. 2009, 2009
 *
 * THIS FILE IS PROVIDED UNDER THE TERMS OF THE ECLIPSE PUBLIC LICENSE
 * ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
 * CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
 *
 * You can obtain a current copy of the Eclipse Public License from
 * http://www.opensource.org/licenses/eclipse-1.0.php
 *
 * Author:       Tyrel Datwyler  <tyreld@us.ibm.com>
 * Contributors: 
 *
 * Description:
 * Plugin helper API for collecting Virtualization Metrics via Libvirt
 *
 */

// #define DEBUG

#include "metricVirt.h"

#include <commutil.h>

#include <libvirt/libvirt.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static virConnectPtr conn;
static int hyp_type;

static time_t last_time_sampled;

int connectHypervisor(int type)
{
	virConnectPtr tconn;
	const char * uri;
	
	switch (type) {
	case XEN_HYP:
		uri = "xen:///";
		break;
	case KVM_HYP:
		uri = "qemu:///system";
		break;
	default:
		return 0;
	}
	
	tconn = virConnectOpen(uri);
	
	if (tconn) {
		conn = tconn;
		hyp_type = type;
	}
	
	return (tconn ? 1 : 0);
}

static int collectNodeStats()
{
	virNodeInfo ninfo;

#ifdef DEBUG
	fprintf(stderr, "collectNodeStats()\n");
#endif

	node_statistics.num_domains = virConnectNumOfDomains(conn);
	if (node_statistics.num_domains < 0)
		return -1;

	node_statistics.free_memory = virNodeGetFreeMemory(conn) / 1024;
	if (virNodeGetInfo(conn, &ninfo)) {
		return -1;
	}
	node_statistics.total_memory = ninfo.memory;

#ifdef DEBUG
    fprintf(stderr, "--- %s(%i) : total_memory %lld   free_memory %lld\n",
	    __FILE__, __LINE__, node_statistics.total_memory, node_statistics.free_memory);
#endif	

	return 0;	
}

static int collectDomainStats()
{
	virDomainPtr domain;
	virDomainInfo dinfo;
	int * ids;
	int i;

#ifdef DEBUG
	fprintf(stderr, "collectDomainStats()\n");
#endif

    // only update the statistics if this function was called more than 10 seconds ago
    if ((time(NULL) - last_time_sampled) < 10) {
#ifdef DBUG
	fprintf(stderr, "parseXm called too frequently\n");
#endif
		return 0;
    } else {
		node_statistics.num_domains = 0;	// reset number of domains
		last_time_sampled = time(NULL);
    }

	if (collectNodeStats())
		return -1;
		
	ids = malloc(sizeof(ids) * node_statistics.num_domains);

	if ((node_statistics.num_domains = virConnectListDomains(conn, ids, node_statistics.num_domains)) < 1)
		return -1;

#ifdef DEBUG
	fprintf(stderr, "--- %s(%i) : num_domains  %d\n", __FILE__, __LINE__, node_statistics.num_domains);
#endif

	for (i = 0; i < node_statistics.num_domains; ids++, i++) {
		domain = virDomainLookupByID(conn, *ids);
		domain_statistics.domain_id[i] = *ids;
		domain_statistics.domain_name[i] = strdup(virDomainGetName(domain));
		
		virDomainGetInfo(domain, &dinfo);
		
		domain_statistics.claimed_memory[i] = dinfo.memory;
		domain_statistics.max_memory[i] = dinfo.maxMem;
		domain_statistics.cpu_time[i] = ((float) dinfo.cpuTime) / 1000000000;
		domain_statistics.vcpus[i] = dinfo.nrVirtCpu;
		
#ifdef DEBUG
	fprintf(stderr, "--- %s(%i) : %s (%d)\n\t claimed %lu  max %lu\n\t time %f  cpus %hu\n",
		__FILE__, __LINE__, domain_statistics.domain_name[i], *ids, dinfo.memory, dinfo.maxMem,
		domain_statistics.cpu_time[i], dinfo.nrVirtCpu);
#endif
		
		virDomainFree(domain);
	}
	
	return 0;
}

/* ---------------------------------------------------------------------------*/
/* _Internal_CPUTime                                                          */
/* (divided by number of virtual CPUs)                                        */
/* ---------------------------------------------------------------------------*/

int virtMetricRetrCPUTime(int mid, MetricReturner mret)
{
    MetricValue *mv = NULL;

#ifdef DEBUG
    fprintf(stderr, "--- %s(%i) : Retrieving Xen CPUTime\n",
	    __FILE__, __LINE__);
#endif

    collectDomainStats();

    if (mret == NULL) {
#ifdef DEBUG
	fprintf(stderr, "Returner pointer is NULL\n");
#endif
    } else {
#ifdef DEBUG
	fprintf(stderr,
		"--- %s(%i) : Sampling for metric ExternalViewTotalCPUTimePercentage %d\n",
		__FILE__, __LINE__, mid);
#endif

	int i;

#ifdef DEBUG
	fprintf(stderr, "--- %s(%i) : num_domains %d\n",
		__FILE__, __LINE__, node_statistics.num_domains);
#endif
	for (i = 0; i < node_statistics.num_domains; i++) {

	    mv = calloc(1, sizeof(MetricValue) +
			sizeof(float) +
			strlen(domain_statistics.domain_name[i]) + 1);

	    if (mv) {
		mv->mvId = mid;
		mv->mvTimeStamp = time(NULL);
		mv->mvDataType = MD_FLOAT32;
		mv->mvDataLength = sizeof(float);
		mv->mvData = (char *) mv + sizeof(MetricValue);
		*(float *) mv->mvData = htonf(domain_statistics.cpu_time[i]
					      / domain_statistics.vcpus[i]);

		mv->mvResource = (char *) mv + sizeof(MetricValue)
		    + sizeof(float);
		strcpy(mv->mvResource, domain_statistics.domain_name[i]);
		mret(mv);
	    }

	}
	return 1;
    }
    return -1;
}


/* ---------------------------------------------------------------------------*/
/* TotalCPUTime                                                     */
/* (not divided by number of virtual CPUs)                                    */
/* ---------------------------------------------------------------------------*/

int virtMetricRetrTotalCPUTime(int mid, MetricReturner mret)
{
    MetricValue *mv = NULL;

#ifdef DEBUG
    fprintf(stderr, "--- %s(%i) : Retrieving Xen CPUTime\n",
	    __FILE__, __LINE__);
#endif

    collectDomainStats();

    if (mret == NULL) {
#ifdef DEBUG
	fprintf(stderr, "Returner pointer is NULL\n");
#endif
    } else {
#ifdef DEBUG
	fprintf(stderr,
		"--- %s(%i) : Sampling for metric ExternalViewTotalCPUTimePercentage %d\n",
		__FILE__, __LINE__, mid);
#endif

	int i;

#ifdef DEBUG
	fprintf(stderr, "--- %s(%i) : num_domains %d\n",
		__FILE__, __LINE__, node_statistics.num_domains);
#endif
	for (i = 0; i < node_statistics.num_domains; i++) {

	    mv = calloc(1, sizeof(MetricValue) +
			sizeof(unsigned long long) +
			strlen(domain_statistics.domain_name[i]) + 1);

	    if (mv) {
		mv->mvId = mid;
		mv->mvTimeStamp = time(NULL);
		mv->mvDataType = MD_UINT64;
		mv->mvDataLength = sizeof(unsigned long long);
		mv->mvData = (char *) mv + sizeof(MetricValue);
		*(unsigned long long *) mv->mvData
		    =
		    htonll((unsigned long long) (domain_statistics.
						 cpu_time[i] * 1000));

#ifdef DEBUG
		fprintf(stderr,
			"--- %s(%i) : metric_id %d metric_value %f \n",
			__FILE__, __LINE__, mid, *(float *) mv->mvData);
#endif

		mv->mvResource = (char *) mv + sizeof(MetricValue)
		    + sizeof(unsigned long long);
		strcpy(mv->mvResource, domain_statistics.domain_name[i]);
		mret(mv);
	    }

	}
	return 1;
    }
    return -1;
}


/* ---------------------------------------------------------------------------*/
/* ActiveVirtualProcessors                                                    */
/* ---------------------------------------------------------------------------*/

int virtMetricRetrActiveVirtualProcessors(int mid, MetricReturner mret)
{
    MetricValue *mv = NULL;

#ifdef DEBUG
    fprintf(stderr,
	    "--- %s(%i) : Retrieving Xen ActiveVirtualProcessors metric\n",
	    __FILE__, __LINE__);
#endif

    collectDomainStats();

    if (mret == NULL) {
#ifdef DEBUG
	fprintf(stderr, "Returner pointer is NULL\n");
#endif
    } else {
#ifdef DEBUG
	fprintf(stderr,
		"--- %s(%i) : Sampling for metric ActiveVirtualProcessors %d\n",
		__FILE__, __LINE__, mid);
#endif

	int i;

#ifdef DEBUG
	fprintf(stderr, "--- %s(%i) : num_domains %d\n",
		__FILE__, __LINE__, node_statistics.num_domains);
#endif
	for (i = 0; i < node_statistics.num_domains; i++) {

	    mv = calloc(1, sizeof(MetricValue) +
			sizeof(float) +
			strlen(domain_statistics.domain_name[i]) + 1);

	    if (mv) {
		mv->mvId = mid;
		mv->mvTimeStamp = time(NULL);
		mv->mvDataType = MD_FLOAT32;
		mv->mvDataLength = sizeof(float);
		mv->mvData = (char *) mv + sizeof(MetricValue);
		*(float *) mv->mvData = (float) domain_statistics.vcpus[i];

		mv->mvResource = (char *) mv + sizeof(MetricValue)
		    + sizeof(float);
		strcpy(mv->mvResource, domain_statistics.domain_name[i]);
		mret(mv);
	    }
	}
	return 1;
    }
    return -1;
}


/* ---------------------------------------------------------------------------*/
/* Internal aggregate for raw memory related metrics                          */
/* ---------------------------------------------------------------------------*/
int virtMetricRetrInternalMemory(int mid, MetricReturner mret)
{
    MetricValue *mv = NULL;

#ifdef DEBUG
    fprintf(stderr,
	    "--- %s(%i) : Retrieving Xen MaximumPhysicalMemoryAllocatedToVirtualSystem metric\n",
	    __FILE__, __LINE__);
#endif

    collectDomainStats();

    if (mret == NULL) {
#ifdef DEBUG
	fprintf(stderr, "Returner pointer is NULL\n");
#endif
    } else {
	int i;

#ifdef DEBUG
	fprintf(stderr, "--- %s(%i) : num_domains %d\n",
		__FILE__, __LINE__, node_statistics.num_domains);
#endif

	char buf[70];		// 3 unsigned long, max 20 characters each

	for (i = 0; i < node_statistics.num_domains; i++) {
	    memset(buf,0,sizeof(buf));
	    sprintf(buf,
		    "%lld:%lld:%lld",
		    domain_statistics.claimed_memory[i],
		    domain_statistics.max_memory[i], node_statistics.total_memory);
#ifdef DEBUG
	    fprintf(stderr, "%s internal memory metric: %s size: %d\n",
		    domain_statistics.domain_name[i], buf, strlen(buf));
#endif
	    mv = calloc(1, sizeof(MetricValue) +
			strlen(buf) + strlen(domain_statistics.domain_name[i]) + 2);

	    if (mv) {
		mv->mvId = mid;
		mv->mvTimeStamp = time(NULL);
		mv->mvDataType = MD_STRING;
		mv->mvDataLength = (strlen(buf)+1);
		mv->mvData = (char *) mv + sizeof(MetricValue);
		strncpy(mv->mvData, buf, strlen(buf));

		mv->mvResource = (char *) mv + sizeof(MetricValue)
		  + (strlen(buf)+1);
		strcpy(mv->mvResource, domain_statistics.domain_name[i]);
		mret(mv);
	    }
	}
	return 1;
    }
    return -1;
}


/* ---------------------------------------------------------------------------*/
/* HostFreePhysicalMemory                                                     */
/* ---------------------------------------------------------------------------*/

int virtMetricRetrHostFreePhysicalMemory(int mid, MetricReturner mret)
{
    MetricValue *mv = NULL;
    int len;

#ifdef DEBUG
    fprintf(stderr,
	    "--- %s(%i) : Retrieving Xen HostFreePhysicalMemory metric\n",
	    __FILE__, __LINE__);
#endif

    collectDomainStats();

    if (mret == NULL) {
#ifdef DEBUG
	fprintf(stderr, "Returner pointer is NULL\n");
#endif
    } else {
    if	(hyp_type == XEN_HYP) {
    	len = strlen(domain_statistics.domain_name[0]) + 1;
    } else {
    	len = 16;
    }
    
	mv = calloc(1, sizeof(MetricValue) +
		    sizeof(unsigned long long) +
		    len);

	if (mv) {
	    mv->mvId = mid;
	    mv->mvTimeStamp = time(NULL);
	    mv->mvDataType = MD_UINT64;
	    mv->mvDataLength = sizeof(unsigned long long);
	    mv->mvData = (char *) mv + sizeof(MetricValue);
	    *(unsigned long long *) mv->mvData = node_statistics.free_memory;

	    mv->mvResource = (char *) mv + sizeof(MetricValue)
		+ sizeof(unsigned long long);
		if (hyp_type == XEN_HYP) {
	    	strcpy(mv->mvResource, domain_statistics.domain_name[0]);
	    } else {
	    	strcpy(mv->mvResource, "OperatingSystem");
	    }
	    mret(mv);
	}
	return 1;
    }
    return -1;
}

int main(int argc, char ** argv)
{
	if (argc == 2) {
		switch((char)argv[1][0]) {
		case 'x':
			connectHypervisor(XEN_HYP);
			break;
		case 'k':
			connectHypervisor(KVM_HYP);
			break;
		default:
			goto fail;
		}
	} else {
		goto fail;
	}
	
	if (conn) {
		collectDomainStats();
		exit(EXIT_SUCCESS);
	}

fail:
	fprintf(stderr, "usage: %s [xk]\n", argv[0]);
	exit(EXIT_FAILURE);
}
