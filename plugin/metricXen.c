/*
 * $Id: metricXen.c,v 1.13 2010/08/04 23:24:36 tyreld Exp $
 *
 * © Copyright IBM Corp. 2006, 2007, 2009
 *
 * THIS FILE IS PROVIDED UNDER THE TERMS OF THE ECLIPSE PUBLIC LICENSE
 * ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
 * CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
 *
 * You can obtain a current copy of the Eclipse Public License from
 * http://www.opensource.org/licenses/eclipse-1.0.php
 *
 * Author:       Oliver Benke (benke@de.ibm.com
 * Contributors: 
 *
 * Description:
 * Metrics gatherer plugin for the following Xen specific metrics:
 *
 *    TotalCPUTime
 *    ActiveVirtualProcessors
 *    ExternalViewTotalCPUPercentage
 *    PhysicalMemoryAllocatedToVirtualSystem
 *    PartitionClaimedMaximumMemory
 *    HostFreePhysicalMemory
 *    PhysicalMemoryAllocatedToVirtualSystemPercentage
 *    HostMemoryPercentage
 * 
 * plus the following metrics which are only intended for internal usage:
 *    _Internal_CPUTime
 *    _Internal_TotalCPUTime
 *    _Internal_Memory
 *
 */

/* ---------------------------------------------------------------------------*/

// #define DEBUG

#include <mplugin.h>
#include <commutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>

/* ---------------------------------------------------------------------------*/

static MetricDefinition metricDef[7];

/* --- Xen metric retrievers --- */
static MetricRetriever metricRetrCPUTime;
static MetricRetriever metricRetrTotalCPUTime;
static MetricRetriever metricRetrActiveVirtualProcessors;
static MetricRetriever metricRetrInternalMemory;
static MetricRetriever metricRetrHostFreePhysicalMemory;

/* ---------------------------------------------------------------------------*/


/* ---------------------------------------------------------------------------*/
/* internal global variables for this plugin                                  */
/* ---------------------------------------------------------------------------*/

#define MAX_DOMAINS 255

/**
 * number of Xen domains
 */
static unsigned int num_domains;

/** 
 * global statistics, output of "xm list --info"
 */
static struct xen_statistics_type {
    unsigned int domain_id[MAX_DOMAINS];
    char *domain_name[MAX_DOMAINS];
    unsigned long long claimed_memory[MAX_DOMAINS];
    unsigned long long max_memory[MAX_DOMAINS];
    float cpu_time[MAX_DOMAINS];
    unsigned short vcpus[MAX_DOMAINS];
} xen_statistics;

/**
 * used to prevent that "xm list --info" is called too frequently; 
 * currently, it's called at most once every 6 seconds
 */
static time_t last_time_sampled;

/**
 * == 0 : not tested, needs to run parseXmInfo
 * == 1 : is a Xen system
 * == -1: is not a Xen system
 */
static int isXenSystem = 0;

/**
 * "xm info" result: total_memory and free_memory for complete server
 */
static unsigned long long total_memory = 0;
static unsigned long long free_memory = 0;

/* ---------------------------------------------------------------------------*/
/* internal helper functions                                                  */
/* ---------------------------------------------------------------------------*/

/**
 * parse output of the Xen command
 *  "xm list --long"
 * and store the result in global variables
 */
int parseXm();

/**
 * parse output of the Xen command
 *  "xm list --long"
 * and store the result in global variables
 */
int parseXmInfo();


/* ---------------------------------------------------------------------------*/

int _DefinedMetrics(MetricRegisterId * mr,
		    const char *pluginname,
		    size_t * mdnum, MetricDefinition ** md)
{

#ifdef DEBUG
    fprintf(stderr, "retrieving metric definitions\n");
#endif


    parseXmInfo();
#ifdef DEBUG
    fprintf(stderr, "--- %s(%i) : Xen is %s\n",
	    __FILE__, __LINE__,
	    (isXenSystem == 1) ? "active" : "NOT active");
#endif


    if (mr == NULL || mdnum == NULL || md == NULL) {
#ifdef DEBUG
	fprintf(stderr,
		"--- %s(%i) : invalid parameter list\n",
		__FILE__, __LINE__);
#endif
	return -1;
    }

    if (isXenSystem == 1) {
	metricDef[0].mdVersion = MD_VERSION;
	metricDef[0].mdName = "_Internal_CPUTime";
	metricDef[0].mdReposPluginName = "librepositoryXen.so";
	metricDef[0].mdId = mr(pluginname, metricDef[0].mdName);
	metricDef[0].mdSampleInterval = 60;
	metricDef[0].mproc = metricRetrCPUTime;
	metricDef[0].mdeal = free;

	metricDef[1].mdVersion = MD_VERSION;
	metricDef[1].mdName = "_Internal_TotalCPUTime";
	metricDef[1].mdReposPluginName = "librepositoryXen.so";
	metricDef[1].mdId = mr(pluginname, metricDef[1].mdName);
	metricDef[1].mdSampleInterval = 60;
	metricDef[1].mproc = metricRetrTotalCPUTime;
	metricDef[1].mdeal = free;

	metricDef[2].mdVersion = MD_VERSION;
	metricDef[2].mdName = "ActiveVirtualProcessors";
	metricDef[2].mdReposPluginName = "librepositoryXen.so";
	metricDef[2].mdId = mr(pluginname, metricDef[2].mdName);
	metricDef[2].mdSampleInterval = 60;
	metricDef[2].mproc = metricRetrActiveVirtualProcessors;
	metricDef[2].mdeal = free;

	metricDef[3].mdVersion = MD_VERSION;
	metricDef[3].mdName = "_Internal_Memory";
	metricDef[3].mdReposPluginName = "librepositoryXen.so";
	metricDef[3].mdId = mr(pluginname, metricDef[3].mdName);
	metricDef[3].mdSampleInterval = 60;
	metricDef[3].mproc = metricRetrInternalMemory;
	metricDef[3].mdeal = free;

	metricDef[4].mdVersion = MD_VERSION;
	metricDef[4].mdName = "HostFreePhysicalMemory";
	metricDef[4].mdReposPluginName = "librepositoryXen.so";
	metricDef[4].mdId = mr(pluginname, metricDef[4].mdName);
	metricDef[4].mdSampleInterval = 60;
	metricDef[4].mproc = metricRetrHostFreePhysicalMemory;
	metricDef[4].mdeal = free;

	metricDef[5].mdVersion = MD_VERSION;
	metricDef[5].mdName = "_Internal10m_CPUTime";
	metricDef[5].mdReposPluginName = "librepositoryXen.so";
	metricDef[5].mdId = mr(pluginname, metricDef[5].mdName);
	metricDef[5].mdSampleInterval = 600;
	metricDef[5].mproc = metricRetrCPUTime;
	metricDef[5].mdeal = free;

	metricDef[6].mdVersion = MD_VERSION;
	metricDef[6].mdName = "_Internal10m_TotalCPUTime";
	metricDef[6].mdReposPluginName = "librepositoryXen.so";
	metricDef[6].mdId = mr(pluginname, metricDef[6].mdName);
	metricDef[6].mdSampleInterval = 600;
	metricDef[6].mproc = metricRetrTotalCPUTime;
	metricDef[6].mdeal = free;

	*mdnum = 7;
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
    for (i = 0; i < 2; i++) {
	memset((char *) &xen_statistics.domain_name[0],
	       0, MAX_DOMAINS * sizeof(char *));
    }
    return 0;
}


/* ---------------------------------------------------------------------------*/
/* _Internal_CPUTime                                                          */
/* (divided by number of virtual CPUs)                                        */
/* ---------------------------------------------------------------------------*/

int metricRetrCPUTime(int mid, MetricReturner mret)
{
    MetricValue *mv = NULL;

#ifdef DEBUG
    fprintf(stderr, "--- %s(%i) : Retrieving Xen CPUTime\n",
	    __FILE__, __LINE__);
#endif

    parseXm();

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
		__FILE__, __LINE__, num_domains);
#endif
	for (i = 0; i < num_domains; i++) {

	    mv = calloc(1, sizeof(MetricValue) +
			sizeof(float) +
			strlen(xen_statistics.domain_name[i]) + 1);

	    if (mv) {
		mv->mvId = mid;
		mv->mvTimeStamp = time(NULL);
		mv->mvDataType = MD_FLOAT32;
		mv->mvDataLength = sizeof(float);
		mv->mvData = (char *) mv + sizeof(MetricValue);
		*(float *) mv->mvData = htonf(xen_statistics.cpu_time[i]
					      / xen_statistics.vcpus[i]);

		mv->mvResource = (char *) mv + sizeof(MetricValue)
		    + sizeof(float);
		strcpy(mv->mvResource, xen_statistics.domain_name[i]);
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

int metricRetrTotalCPUTime(int mid, MetricReturner mret)
{
    MetricValue *mv = NULL;

#ifdef DEBUG
    fprintf(stderr, "--- %s(%i) : Retrieving Xen CPUTime\n",
	    __FILE__, __LINE__);
#endif

    parseXm();

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
		__FILE__, __LINE__, num_domains);
#endif
	for (i = 0; i < num_domains; i++) {

	    mv = calloc(1, sizeof(MetricValue) +
			sizeof(unsigned long long) +
			strlen(xen_statistics.domain_name[i]) + 1);

	    if (mv) {
		mv->mvId = mid;
		mv->mvTimeStamp = time(NULL);
		mv->mvDataType = MD_UINT64;
		mv->mvDataLength = sizeof(unsigned long long);
		mv->mvData = (char *) mv + sizeof(MetricValue);
		*(unsigned long long *) mv->mvData
		    =
		    htonll((unsigned long long) (xen_statistics.
						 cpu_time[i] * 1000));

#ifdef DEBUG
		fprintf(stderr,
			"--- %s(%i) : metric_id %d metric_value %f \n",
			__FILE__, __LINE__, mid, *(float *) mv->mvData);
#endif

		mv->mvResource = (char *) mv + sizeof(MetricValue)
		    + sizeof(unsigned long long);
		strcpy(mv->mvResource, xen_statistics.domain_name[i]);
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

int metricRetrActiveVirtualProcessors(int mid, MetricReturner mret)
{
    MetricValue *mv = NULL;

#ifdef DEBUG
    fprintf(stderr,
	    "--- %s(%i) : Retrieving Xen ActiveVirtualProcessors metric\n",
	    __FILE__, __LINE__);
#endif

    parseXm();

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
		__FILE__, __LINE__, num_domains);
#endif
	for (i = 0; i < num_domains; i++) {

	    mv = calloc(1, sizeof(MetricValue) +
			sizeof(float) +
			strlen(xen_statistics.domain_name[i]) + 1);

	    if (mv) {
		mv->mvId = mid;
		mv->mvTimeStamp = time(NULL);
		mv->mvDataType = MD_FLOAT32;
		mv->mvDataLength = sizeof(float);
		mv->mvData = (char *) mv + sizeof(MetricValue);
		*(float *) mv->mvData = (float) xen_statistics.vcpus[i];

		mv->mvResource = (char *) mv + sizeof(MetricValue)
		    + sizeof(float);
		strcpy(mv->mvResource, xen_statistics.domain_name[i]);
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
int metricRetrInternalMemory(int mid, MetricReturner mret)
{
    MetricValue *mv = NULL;

#ifdef DEBUG
    fprintf(stderr,
	    "--- %s(%i) : Retrieving Xen MaximumPhysicalMemoryAllocatedToVirtualSystem metric\n",
	    __FILE__, __LINE__);
#endif

    parseXm();

    if (mret == NULL) {
#ifdef DEBUG
	fprintf(stderr, "Returner pointer is NULL\n");
#endif
    } else {
	int i;

#ifdef DEBUG
	fprintf(stderr, "--- %s(%i) : num_domains %d\n",
		__FILE__, __LINE__, num_domains);
#endif

	char buf[70];		// 3 unsigned long, max 20 characters each

	for (i = 0; i < num_domains; i++) {
	    memset(buf,0,sizeof(buf));
	    sprintf(buf,
		    "%lld:%lld:%lld",
		    xen_statistics.claimed_memory[i],
		    xen_statistics.max_memory[i], total_memory);
#ifdef DEBUG
	    fprintf(stderr, "%s internal memory metric: %s size: %d\n",
		    xen_statistics.domain_name[i], buf, strlen(buf));
#endif
	    mv = calloc(1, sizeof(MetricValue) +
			strlen(buf) + strlen(xen_statistics.domain_name[i]) + 2);

	    if (mv) {
		mv->mvId = mid;
		mv->mvTimeStamp = time(NULL);
		mv->mvDataType = MD_STRING;
		mv->mvDataLength = (strlen(buf)+1);
		mv->mvData = (char *) mv + sizeof(MetricValue);
		strncpy(mv->mvData, buf, strlen(buf));

		mv->mvResource = (char *) mv + sizeof(MetricValue)
		  + (strlen(buf)+1);
		strcpy(mv->mvResource, xen_statistics.domain_name[i]);
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

int metricRetrHostFreePhysicalMemory(int mid, MetricReturner mret)
{
    MetricValue *mv = NULL;

#ifdef DEBUG
    fprintf(stderr,
	    "--- %s(%i) : Retrieving Xen HostFreePhysicalMemory metric\n",
	    __FILE__, __LINE__);
#endif

    parseXm();

    if (mret == NULL) {
#ifdef DEBUG
	fprintf(stderr, "Returner pointer is NULL\n");
#endif
    } else {
	mv = calloc(1, sizeof(MetricValue) +
		    sizeof(unsigned long long) +
		    strlen(xen_statistics.domain_name[0]) + 1);

	if (mv) {
	    mv->mvId = mid;
	    mv->mvTimeStamp = time(NULL);
	    mv->mvDataType = MD_UINT64;
	    mv->mvDataLength = sizeof(unsigned long long);
	    mv->mvData = (char *) mv + sizeof(MetricValue);
	    *(unsigned long long *) mv->mvData = free_memory;

	    mv->mvResource = (char *) mv + sizeof(MetricValue)
		+ sizeof(unsigned long long);
	    strcpy(mv->mvResource, xen_statistics.domain_name[0]);
	    mret(mv);
	}
	return 1;
    }
    return -1;
}


/* ---------------------------------------------------------------------------*/
/* parse output of the Xen command                                            */
/*  "xm list --long"                                                          */
/* and store the result in global variables                                   */
/* ---------------------------------------------------------------------------*/
#define BUFFER_MAX 65530

int parseXm()
{
    // only update the statistics if this function was called more than 10 seconds ago
    if ((time(NULL) - last_time_sampled) < 10) {
#ifdef DBUG
	fprintf(stderr, "parseXm called too frequently\n");
#endif
	return 0;
    } else {
	num_domains = 0;	// reset number of domains
	last_time_sampled = time(NULL);
    }

#ifdef DEBUG
    fprintf(stderr, "parseXm\n");
#endif

    FILE *fp;
    char buffer[BUFFER_MAX];
    char *buf_current;
    char *buf_end;
    char *xm_result;
    char *xm_current;
    char *xm_endp;

    fp = popen("xm list --long", "r");
    if (NULL == fp) {
#ifdef DEBUG
	perror("xm list popen");
#endif
	return -1;
    }

    buf_current = (char *) &buffer[0];
    buf_end = (char *) &buffer[BUFFER_MAX-1];

    while (buf_current < buf_end) {
      if (EOF == (signed char)(*buf_current++ = fgetc(fp)))
	    break;
    }
    *buf_current = '\0';

    if (0 == strlen(buffer)) {
	perror("fgets");
    }

    if (pclose(fp) == -1) {
	perror("pclose");
    }

    xm_result = (char *) &buffer[0];
#ifdef DEBUG
    if (NULL != xm_result)
	fprintf(stderr, "--- %s(%i) : xm_result: \n %s\n",
		__FILE__, __LINE__, xm_result);
#endif

    // parse result of "xm list" command above
    while (NULL != (xm_current = strstr(xm_result, "(domain"))) {
	xm_result = strstr(xm_current, "(domid ");
	if (NULL == xm_result)
	    return -1;
   xm_result += strlen("(domid ");
	xm_endp = strstr(xm_result, ")");
	xen_statistics.domain_id[num_domains]
	    = strtol(xm_result, &xm_endp, 10);

	xm_result = strstr(xm_current, "(memory ") + strlen("(memory ");
	xm_endp = strstr(xm_result, ")");
	xen_statistics.claimed_memory[num_domains]
	    = strtol(xm_result, &xm_endp, 10) * 1024;

#ifdef DEBUG
	fprintf(stderr, "--- %s(%i) : memory: %lld\n",
		__FILE__, __LINE__,
		xen_statistics.claimed_memory[num_domains]);
#endif

	xm_result = strstr(xm_current, "(name ") + strlen("(name ");
	xm_endp = strstr(xm_result, ")");

	if (NULL != xen_statistics.domain_name[num_domains])
	    free(xen_statistics.domain_name[num_domains]);
	xen_statistics.domain_name[num_domains] =
	    calloc(1,xm_endp - xm_result + 2);
	strncpy(xen_statistics.domain_name[num_domains], xm_result,
		xm_endp - xm_result);
#ifdef DEBUG
	fprintf(stderr, "--- %s(%i) : domain_name: %s\n",
		__FILE__, __LINE__,
		xen_statistics.domain_name[num_domains]);
#endif

	xm_result = strstr(xm_current, "(maxmem ") + strlen("(maxmem ");
	xm_endp = strstr(xm_result, ")");
	xen_statistics.max_memory[num_domains]
	    = strtol(xm_result, &xm_endp, 10) * 1024;

#ifdef DEBUG
	fprintf(stderr, "--- %s(%i) : maxmem: %lld\n",
		__FILE__, __LINE__,
		xen_statistics.max_memory[num_domains]);
#endif

	xm_result = strstr(xm_current, "(vcpus ") + strlen("(vcpus ");
	if (NULL == xm_result)
	    return -1;
	xm_endp = strstr(xm_result, ")");
	xen_statistics.vcpus[num_domains]
	    = (unsigned short) strtol(xm_result, &xm_endp, 10);

	xm_result = strstr(xm_current, "(cpu_time ") + strlen("(cpu_time ");
	xm_endp = strstr(xm_result, ")");

	xen_statistics.cpu_time[num_domains]
	    = strtod(xm_result, &xm_endp);

#ifdef DEBUG
	fprintf(stderr, "--- %s(%i) : cpu_time: %f\n",
		__FILE__, __LINE__, xen_statistics.cpu_time[num_domains]);
#endif

	num_domains++;
    }

    parseXmInfo();

    return 0;
}


/* ---------------------------------------------------------------------------*/
/* parse output of the Xen command                                            */
/*  "xm info"                                                                 */
/* and store the result in global variables                                   */
/* ---------------------------------------------------------------------------*/

int parseXmInfo()
{

    isXenSystem = -1;

#ifdef DEBUG
    fprintf(stderr, "parseXmInfo\n");
#endif

    FILE *fp;
    char buffer[BUFFER_MAX];
    char *buf_current;
    char *buf_end;
    char *xm_result;
    char *xm_endp;

    fp = popen("xm info", "r");
    if (NULL == fp) {
#ifdef DEBUG
	fprintf(stderr, "not a XEN system\n");
	perror("xm info popen");
#endif
	isXenSystem = -1;
	return 0;
    }

    buf_current = (char *) &buffer[0];
    buf_end = (char *) &buffer[BUFFER_MAX-1];

    while (buf_current < buf_end) {
      if (EOF == (signed char)(*buf_current++ = fgetc(fp)))
	    break;
    }
    *buf_current = '\0';

    if (0 == strlen(buffer)) {
	perror("fgets");
    }

    if (pclose(fp) == -1) {
	perror("pclose");
    }

    xm_result = (char *) &buffer[0];
    if (strlen(xm_result) < 50) {
#ifdef DEBUG
	fprintf(stderr, "not a XEN system\n");
#endif
	isXenSystem = -1;
	return 0;
    } else {
	isXenSystem = 1;
    }

#ifdef DEBUG
    if (NULL != xm_result)
	fprintf(stderr, "--- %s(%i) : xm_result: \n %s\n",
		__FILE__, __LINE__, xm_result);
#endif

    // parse result of "xm info" command above
    xm_result = strstr(xm_result, "total_memory") + strlen("total_memory");
    if (NULL == xm_result)
	return -1;
    xm_result = strstr(xm_result, ":") + 1;
    if (NULL == xm_result)
	return -1;
    xm_endp = xm_result;
    while (!isalpha(*xm_endp)) {
	xm_endp++;
    }
    total_memory = strtol(xm_result, &xm_endp, 10) * 1024;

    xm_result = strstr(xm_result, "free_memory") + strlen("free_memory");
    if (NULL == xm_result)
	return -1;
    xm_result = strstr(xm_result, ":") + 1;
    if (NULL == xm_result)
	return -1;
    xm_endp = xm_result;
    while (!isalpha(*xm_endp)) {
	xm_endp++;
    }
    free_memory = strtol(xm_result, &xm_endp, 10) * 1024;
#ifdef DEBUG
    fprintf(stderr, "--- %s(%i) : total_memory %lld   free_memory %lld\n",
	    __FILE__, __LINE__, total_memory, free_memory);
#endif
    return 0;
}

/* ---------------------------------------------------------------------------*/
/*                    end of metricXen.c                                      */
/* ---------------------------------------------------------------------------*/
