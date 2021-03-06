/*
 * $Id: metricVirt.c,v 1.21 2012/04/03 20:19:26 tyreld Exp $
 *
 * (C) Copyright IBM Corp. 2009, 2011
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
#include <mlog.h>

#include <libvirt/libvirt.h>
#include <libvirt/virterror.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PIDDIR "/var/run/libvirt/qemu/"
#define L_piddir 22
#define PROC "/proc/"
#define TASK "/task/"
#define SCHED "/sched"
#define L_sched 32
#define bufsize 4096

static virConnectPtr conn;
static int hyp_type = NO_HYP;
static int err_fn_set = 0;

static time_t last_time_sampled;

static void logHypervisorErrors(void *userData, virErrorPtr err)
{
    m_log(M_INFO, M_SHOW, "libvirt error: %s\n", err->message);
}

static int connectHypervisor()
{
	virConnectPtr tconn;
	const char * uri;
	
	switch (hyp_type) {
	case XEN_HYP:
		uri = "xen:///";
		break;
	case KVM_HYP:
		uri = "qemu:///system";
		break;
	default:
		return VIRT_FAIL;
	}
	
	tconn = virConnectOpen(uri);
	
	if (tconn) {
		conn = tconn;
	} else {
        m_log(M_ERROR, M_SHOW, "Failed to open connection with libvirtd on %s\n", uri);
    }

	return (tconn ? VIRT_SUCCESS : VIRT_FAIL);
}

int testHypervisor(int type) {
    int tconn = VIRT_FAIL;

    /* Log failed libvirt calls to syslog */
    if (!err_fn_set) {
        virSetErrorFunc(NULL, logHypervisorErrors);
        err_fn_set = 1;
    }

    /* If no hypervisor type defined yet try and connect */
    if (hyp_type == NO_HYP) {
        hyp_type = type;
        tconn = connectHypervisor();

        if (tconn == VIRT_FAIL) {
            hyp_type = NO_HYP;
            m_log(M_INFO, M_QUIET, "No support for hypervisor type=%d\n", type);
        } else {
            m_log(M_INFO, M_QUIET, "Found support for hypervisor type=%d\n", type);
            virConnectClose(conn);
        }
    }

    return tconn;
}


/* ---------------------------------------------------------------------------*/
/* parseDomainXML                                                             */
/* parse domain XML and collect block io stats                                */
/* ---------------------------------------------------------------------------*/
static struct vdisk_type *parseDomainXML(virDomainPtr domain, int active)
{
    char *cur;
    char *end;
    char *temp;
    virDomainBlockInfo blkinfo;
    virDomainBlockStatsStruct blkstats;
    struct vdisk_type *disk = NULL;
    struct vdisk_type *head = NULL;
    int parse = 0;
    int type = 0;

    parse = 1;
    cur = virDomainGetXMLDesc(domain, 0);

    while (parse && cur) {
	if ((cur = strstr(cur, "<disk"))) {
	    cur = strstr(cur, "type=\'");
	    cur = cur + 6;

	    switch (*cur) {
	    case 'f':
		type = 0;
		break;
	    case 'b':
		type = 1;
		break;
	    default:
		continue;
	    }

	    end = strstr(cur, "</disk");

            /* check that we found a 'source' tag and it is part of the current disk block */
	    temp = strstr(cur, "<source");
	    if (!temp || temp > end) {
		continue;
	    }

	    cur = temp;

	    if (type) {
		cur = strstr(cur, "dev=\'");
		cur = cur + 5;
	    } else {
		cur = strstr(cur, "file=\'");
		cur = cur + 6;
	    }

	    end = strstr(cur, "\'");

	    if (!disk) {
		disk = malloc(sizeof(struct vdisk_type));
		disk->next = NULL;
		head = disk;
	    } else {
		disk->next = malloc(sizeof(struct vdisk_type));
		disk = disk->next;
		disk->next = NULL;
	    }

	    disk->source = malloc(end - cur + 1);
	    disk->source = strncpy(disk->source, cur, end - cur);
	    disk->source[end - cur] = 0;

	    cur = strstr(cur, "<target");
	    cur = strstr(cur, "dev=\'");
	    cur = cur + 5;

	    end = strstr(cur, "\'");

	    disk->target = malloc(end - cur + 1);
	    disk->target = strncpy(disk->target, cur, end - cur);
	    disk->target[end - cur] = 0;

            disk->read = 0;
            disk->write = 0;
            disk->capacity = 0;


	    if (!virDomainGetBlockInfo(domain, disk->source, &blkinfo, 0)) {
        	disk->capacity = blkinfo.capacity;
            }

            /* virDomainBlockStats only works on running domains */
            if (active && (!virDomainBlockStats(domain, 
                                      disk->target, 
                                      &blkstats,
				      sizeof(virDomainBlockStatsStruct)))) {

	        /* Convert to Kilobytes */
	        disk->read = blkstats.rd_bytes / 1024;
	        disk->write = blkstats.wr_bytes / 1024;
            }
	} else {
	    parse = 0;
	}
    }

    return head;
}

static void freeBlkIOData(int domains)
{
    struct vdisk_type * cur, * head;
    int i;
    
    for (i = 0; i < domains; i++) {
        head = domain_statistics.blkio[i];
        
        while (head) {
            cur = head;
            head = cur->next;
            free(cur);
        }

        domain_statistics.blkio[i] = NULL;
    }
}
            

/* ---------------------------------------------------------------------------*/
/* collectDomainSchedStats                                                    */
/* get scheduler statistics for a given domain                                */
/* ---------------------------------------------------------------------------*/
static void collectDomainSchedStats(int cnt)
{
    FILE * fd = NULL;
    char * pidfile = NULL;
    char * tidfile = NULL;
    char tmpfile[L_tmpnam];
    char cmdbuf[128];
    char buf[bufsize];
    int * tids = NULL;
    int pid = 0;
    int i;
    
    /* default stats to 0 in case of error */
	domain_statistics.cpu_used[cnt] = 0;
	domain_statistics.cpu_ready[cnt] = 0;

    /* open libvirts pid file to obtain vm pid */
    pidfile = malloc(sizeof(char *) * 
        (strlen(domain_statistics.domain_name[cnt]) + L_piddir + 4 + 1));
    sprintf(pidfile, "%s%s.pid", PIDDIR, domain_statistics.domain_name[cnt]);
    
    if ((fd = fopen(pidfile, "r")) != NULL) {
        if (fgets(buf, bufsize, fd) != NULL) {
            sscanf(buf, "%d", &pid);
        }
        fclose(fd);
    }
    
    free(pidfile);
    
    /* determine thread ids for each vcpu via ps */
    if (pid) {
        if (tmpnam(tmpfile)) {
            sprintf(cmdbuf, "ps --no-headers -p %d -Lo lwp > %s", pid, tmpfile);
            if (system(cmdbuf) == 0) {
                if ((fd = fopen(tmpfile, "r")) != NULL) {
                    /* ignore master thread (vm pid) */
                    fgets(buf, bufsize, fd);
                    
                    tids = malloc(sizeof(int *) * domain_statistics.vcpus[cnt]);
                    
                    for (i = 0; i < domain_statistics.vcpus[cnt]; i++) {
                        fgets(buf, bufsize, fd);
                        sscanf(buf, "%d", &tids[i]);
                    }                  
                    fclose(fd);
                }
            }
            remove(tmpfile);
        }
    }
    
    /* retrieve scheduler stats for each vcpu/tid */
    if (tids) {
        tidfile = malloc(sizeof(char *) * (L_sched + 1));
        
        /* for each vcpu/tid grab stats from /proc/$pid/task/$tid/sched */
        for (i = 0; i < domain_statistics.vcpus[cnt]; i++) {
            float used, ready;
            
            if (tmpnam(tmpfile)) {
                sprintf(tidfile, "%s%d%s%d%s", PROC, pid, TASK, tids[i], SCHED);
                
                /* interested in se.sum_exec_runtime and se.wait_sum */           
                sprintf(cmdbuf, "cat %s | awk '/exec_runtime/ || /wait_sum/ {print $3}' > %s",
                    tidfile, tmpfile);
                
                /* stats are in floating point ms, convert to microseconds */
                if (system(cmdbuf) == 0) {
                    if ((fd = fopen(tmpfile, "r")) != NULL) {
                        fgets(buf, bufsize, fd);
                        sscanf(buf, "%f", &used);
                        used = used * 1000;
                        domain_statistics.cpu_used[cnt] += used;
                        
                        fgets(buf, bufsize, fd);
                        sscanf(buf, "%f", &ready);
                        ready = ready * 1000;
                        domain_statistics.cpu_ready[cnt] += ready;
                        
                        fclose(fd);
                    }
                }
                remove(tmpfile);
            }
            
        }
        
        /* Average the sum of all stats across number of vcpus */
        domain_statistics.cpu_used[cnt] = domain_statistics.cpu_used[cnt] / domain_statistics.vcpus[cnt];
        domain_statistics.cpu_ready[cnt] = domain_statistics.cpu_ready[cnt] / domain_statistics.vcpus[cnt];
        
        free(tidfile);
        free(tids);
    }
}

/* ---------------------------------------------------------------------------*/
/* collectNodeStats                                                           */
/* get node statistics from libvirt API                                       */
/* ---------------------------------------------------------------------------*/
static int collectNodeStats()
{
	virNodeInfo ninfo;
    int num_dom;

#ifdef DEBUG
	fprintf(stderr, "collectNodeStats()\n");
#endif

	num_dom = virConnectNumOfDomains(conn);
    if (num_dom < 0)
		return VIRT_FAIL;

    node_statistics.num_active_domains = num_dom;

	num_dom = virConnectNumOfDefinedDomains(conn);
    if (num_dom < 0)
		return VIRT_FAIL;
    
    node_statistics.num_inactive_domains = num_dom;

	node_statistics.total_domains = node_statistics.num_active_domains
			                        + node_statistics.num_inactive_domains;

	node_statistics.free_memory = virNodeGetFreeMemory(conn) / 1024;
	if (virNodeGetInfo(conn, &ninfo)) {
		return VIRT_FAIL;
	}
	node_statistics.total_memory = ninfo.memory;

#ifdef DEBUG
    fprintf(stderr, "--- %s(%i) : total_memory %lld   free_memory %lld\n",
	    __FILE__, __LINE__, node_statistics.total_memory, node_statistics.free_memory);
#endif	

	return VIRT_SUCCESS;
}

/* ---------------------------------------------------------------------------*/
/* collectDomainStats                                                         */
/* get domain statistics from libvirt API                                     */
/* ---------------------------------------------------------------------------*/
static int collectDomainStats()
{
	virDomainPtr domain;
	virDomainInfo dinfo;
	char **defdomlist = NULL;
	int  * ids, *ids_ptr;
	size_t cnt, j;
        int num_dom;

#ifdef DEBUG
	fprintf(stderr, "collectDomainStats()\n");
#endif
    
    if (connectHypervisor())
        return VIRT_FAIL;

    // only update the statistics if this function was called more than 10 seconds ago
    if ((time(NULL) - last_time_sampled) < 10) {
#ifdef DBUG
	fprintf(stderr, "parseXm called too frequently\n");
#endif

        virConnectClose(conn);
		return VIRT_NOUPD;
    } else {
        /* free previous running domain vdisk data */
        freeBlkIOData(node_statistics.total_domains);
        
        /* reset domain numbers */
		node_statistics.num_active_domains = 0;
		node_statistics.num_inactive_domains = 0;
		node_statistics.total_domains = 0;
		last_time_sampled = time(NULL);
    }

	if (collectNodeStats()) {
        virConnectClose(conn);
		return VIRT_FAIL;
    }

	/* no domains reported */
	if (node_statistics.total_domains == 0) {
        virConnectClose(conn);
		return VIRT_SUCCESS;
    }

	/*
	 *  get statistics from active domains
	 */
	ids = malloc(sizeof(*ids) * node_statistics.num_active_domains);
	if (ids == NULL) {
        virConnectClose(conn);
		return VIRT_FAIL;
    } else {
		ids_ptr=ids;
    }

	if ((num_dom = virConnectListDomains(conn, ids_ptr,
			                     node_statistics.num_active_domains)) < 0)
	{
        virConnectClose(conn);
		return VIRT_FAIL;
	}
        node_statistics.num_active_domains = num_dom;

#ifdef DEBUG
	fprintf(stderr, "--- %s(%i) : num_active_domains  %d\n", __FILE__, __LINE__, node_statistics.num_active_domains);
#endif

	for (j = 0, cnt = 0; j < node_statistics.num_active_domains; j++, ids_ptr++)
	{
		// On the small chance a domain was shut down since the last call to virConnectListDomains(),
		// the domain id will be invalid and we will get a null ptr here. If so, skip it.
		if (!(domain = virDomainLookupByID(conn, *ids_ptr)))
			continue;

		domain_statistics.domain_id[cnt] = *ids_ptr;
		
		domain_statistics.domain_name[cnt] = realloc(domain_statistics.domain_name[cnt], 
                    sizeof(char *) * (strlen(virDomainGetName(domain)) + 1));

        strcpy(domain_statistics.domain_name[cnt],virDomainGetName(domain));

		virDomainGetInfo(domain, &dinfo);
		
		domain_statistics.claimed_memory[cnt] = dinfo.memory;
		domain_statistics.max_memory[cnt] = dinfo.maxMem;
		domain_statistics.cpu_time[cnt] = ((float) dinfo.cpuTime) / 1000000000;
		domain_statistics.vcpus[cnt] = dinfo.nrVirtCpu;
		domain_statistics.state[cnt] = dinfo.state;
		
		collectDomainSchedStats(cnt);
		domain_statistics.blkio[cnt] = parseDomainXML(domain, 1);
		
#ifdef DEBUG
	fprintf(stderr, "--- %s(%i) : %s (%d)\n\t claimed %lu  max %lu\n\t time %f  cpus %hu\n",
		__FILE__, __LINE__, domain_statistics.domain_name[cnt], *ids_ptr, dinfo.memory, dinfo.maxMem,
		domain_statistics.cpu_time[cnt], dinfo.nrVirtCpu);
#endif
		
		virDomainFree(domain);
		cnt++;
	} /* end for */
	
	// Refresh num_active_domains to the number we actually found.
	node_statistics.num_active_domains = cnt;

	free(ids);

	/*
	 *  get statistics from inactive domains
	 */
	defdomlist = malloc(sizeof(*defdomlist) * node_statistics.num_inactive_domains);
	if (defdomlist == NULL) {
        virConnectClose(conn);
		return VIRT_FAIL;
    }
    
	if ((num_dom = virConnectListDefinedDomains(conn, defdomlist,
	                                       node_statistics.num_inactive_domains)) < 0)
	{
        virConnectClose(conn);
		return VIRT_FAIL;
	}
        node_statistics.num_inactive_domains = num_dom;

	for (j = 0 ; j < node_statistics.num_inactive_domains; j++, cnt++ )
	{
		domain = virDomainLookupByName(conn, *(defdomlist + j));
		domain_statistics.domain_name[cnt] = realloc(domain_statistics.domain_name[cnt], 
                    sizeof(char *) * (strlen(*(defdomlist + j)) + 1));
		
        strcpy(domain_statistics.domain_name[cnt], *(defdomlist + j));

		virDomainGetInfo(domain, &dinfo);

		domain_statistics.claimed_memory[cnt] = 0;
		domain_statistics.max_memory[cnt] = dinfo.maxMem;
		domain_statistics.cpu_time[cnt] = ((float) dinfo.cpuTime) / 1000000000;
		domain_statistics.vcpus[cnt] = 0;
		domain_statistics.state[cnt] = dinfo.state;
		domain_statistics.cpu_used[cnt] = 0;
		domain_statistics.cpu_ready[cnt] = 0;

                domain_statistics.blkio[cnt] = parseDomainXML(domain, 0);
		virDomainFree(domain);

		/* free strdup'ed memory */
		free(*(defdomlist + j));
	} /* end for */

	node_statistics.total_domains = node_statistics.num_active_domains
			                        + node_statistics.num_inactive_domains;
	free(defdomlist);

    virConnectClose(conn);

	return VIRT_SUCCESS;
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

    if (collectDomainStats() == VIRT_FAIL)
        return -1;

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

	size_t i;

#ifdef DEBUG
	fprintf(stderr, "--- %s(%i) : num_active_domains %d\n",
		__FILE__, __LINE__, node_statistics.num_active_domains);
#endif
	for (i = 0; i < node_statistics.total_domains; i++) {

	    mv = calloc(1, sizeof(MetricValue) +
			sizeof(float) +
			strlen(domain_statistics.domain_name[i]) + 1);

	    if (mv) {
		mv->mvId = mid;
		mv->mvTimeStamp = time(NULL);
		mv->mvDataType = MD_FLOAT32;
		mv->mvDataLength = sizeof(float);
		mv->mvData = (char *) mv + sizeof(MetricValue);
        
        if (i < node_statistics.num_active_domains) {
		    *(float *) mv->mvData = htonf(domain_statistics.cpu_time[i]
					      / domain_statistics.vcpus[i]);
        } else {
            *(float *) mv->mvData = 0;
        }

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

    if (collectDomainStats() == VIRT_FAIL)
        return -1;

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

	size_t i;

#ifdef DEBUG
	fprintf(stderr, "--- %s(%i) : num_active_domains %d\n",
		__FILE__, __LINE__, node_statistics.num_active_domains);
#endif
	for (i = 0; i < node_statistics.total_domains; i++) {

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

    if (collectDomainStats() == VIRT_FAIL)
        return -1;

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

	size_t i;

#ifdef DEBUG
	fprintf(stderr, "--- %s(%i) : num_active_domains %d\n",
		__FILE__, __LINE__, node_statistics.num_active_domains);
#endif
	for (i = 0; i < node_statistics.total_domains; i++) {

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

    if (collectDomainStats() == VIRT_FAIL)
        return -1;

    if (mret == NULL) {
#ifdef DEBUG
	fprintf(stderr, "Returner pointer is NULL\n");
#endif
    } else {
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "--- %s(%i) : num_active_domains %d\n",
		__FILE__, __LINE__, node_statistics.num_active_domains);
#endif

	char buf[70];		// 3 unsigned long, max 20 characters each

	for (i = 0; i < node_statistics.total_domains; i++) {
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

    if (collectDomainStats() == VIRT_FAIL)
        return -1;

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


/* ---------------------------------------------------------------------------*/
/* VirtualSystemState                                                        */
/* ---------------------------------------------------------------------------*/

int virtMetricRetrVirtualSystemState(int mid, MetricReturner mret)
{
    MetricValue *mv = NULL;

#ifdef DEBUG
    fprintf(stderr,
	    "--- %s(%i) : Retrieving kvm VirtualSystemState metric\n",
	    __FILE__, __LINE__);
#endif

    if (collectDomainStats() == VIRT_FAIL)
        return -1;

    if (mret == NULL) {
#ifdef DEBUG
    	fprintf(stderr, "Returner pointer is NULL\n");
#endif
    } else {
#ifdef DEBUG
    	fprintf(stderr,
    			"--- %s(%i) : Sampling for metric VirtualSystemState %d\n",
    			__FILE__, __LINE__, mid);
#endif

        size_t i;

#ifdef DEBUG
    	fprintf(stderr, "--- %s(%i) : total_domains %d\n",
    			__FILE__, __LINE__, node_statistics.total_domains);
#endif
    	for (i = 0; i < node_statistics.total_domains; i++) {

    		mv = calloc(1, sizeof(MetricValue) +
    				sizeof(unsigned) +
    				strlen(domain_statistics.domain_name[i]) + 1);

    		if (mv) {
    			mv->mvId = mid;
    			mv->mvTimeStamp = time(NULL);
    			mv->mvDataType = MD_UINT32;
    			mv->mvDataLength = sizeof(unsigned);
    			mv->mvData = (char *) mv + sizeof(MetricValue);
    			*(unsigned *) mv->mvData = (unsigned) domain_statistics.state[i];

    			mv->mvResource = (char *) mv + sizeof(MetricValue)
		    		+ sizeof(unsigned);
    			strcpy(mv->mvResource, domain_statistics.domain_name[i]);
    			mret(mv);
    		}
    	}
    	return 1;
    }
    return -1;
}

/* ----------------------------------------------------------------------*/
/* Scheduler Statistic Metrics                                           */
/* ----------------------------------------------------------------------*/

int virtMetricRetrCPUUsedTimeCounter(int mid, MetricReturner mret)
{
    MetricValue *mv = NULL;

#ifdef DEBUG
    fprintf(stderr,
            "--- %s(%i) : Retrieving KVM Scheduler CPUUsedTimeCounter\n",
            __FILE__, __LINE__);
#endif

    if (collectDomainStats() == VIRT_FAIL)
        return -1;

    if (mret == NULL) {
#ifdef DEBUG
        fprintf(stderr, 
                "--- %s(%i) : Returner pointer is NULL\n", 
                __FILE__, __LINE__);
#endif
    } else {
#ifdef DEBUG
        fprintf(stderr,
                "--- %s(%i) : Sampling for Scheduler CPUUsedTimeCounter metric\n",
                __FILE__, __LINE__);
#endif

        size_t i;

#ifdef DEBUG
        fprintf(stderr,
                "--- %s(%i) : num_active_domains %d\n",
                __FILE__, __LINE__, node_statistics.num_active_domains);
#endif

        for (i = 0; i < node_statistics.total_domains; i++) {

            mv = calloc(1, sizeof(MetricValue) +
                    sizeof(unsigned long long) +
                    strlen(domain_statistics.domain_name[i]) + 1);

            if (mv) {
                mv->mvId = mid;
                mv->mvTimeStamp = time(NULL);
                mv->mvDataType = MD_UINT64;
                mv->mvDataLength = sizeof(unsigned long long);
                mv->mvData = (char *) mv + sizeof(MetricValue);
                *(unsigned long long *) mv->mvData = htonll(domain_statistics.cpu_used[i]);
                mv->mvResource = (char *) mv + sizeof(MetricValue) + sizeof(unsigned long long);
                strcpy(mv->mvResource, domain_statistics.domain_name[i]);
                mret(mv);
            }
        }

        return 1;
    }

    return -1;
}

int virtMetricRetrCPUReadyTimeCounter(int mid, MetricReturner mret)
{
    MetricValue *mv = NULL;

#ifdef DEBUG
    fprintf(stderr,
            "--- %s(%i) : Retrieving KVM Scheduler CPUReadyTimeCounter\n",
            __FILE__, __LINE__);
#endif

    if (collectDomainStats() == VIRT_FAIL)
        return -1;

    if (mret == NULL) {
#ifdef DEBUG
        fprintf(stderr, 
                "--- %s(%i) : Returner pointer is NULL\n", 
                __FILE__, __LINE__);
#endif
    } else {
#ifdef DEBUG
        fprintf(stderr,
                "--- %s(%i) : Sampling for Scheduler CPUReadyTimeCounter metric\n",
                __FILE__, __LINE__);
#endif

        size_t i;

#ifdef DEBUG
        fprintf(stderr,
                "--- %s(%i) : num_active_domains %d\n",
                __FILE__, __LINE__, node_statistics.num_active_domains);
#endif

        for (i = 0; i < node_statistics.total_domains; i++) {

            mv = calloc(1, sizeof(MetricValue) +
                    sizeof(unsigned long long) +
                    strlen(domain_statistics.domain_name[i]) + 1);

            if (mv) {
                mv->mvId = mid;
                mv->mvTimeStamp = time(NULL);
                mv->mvDataType = MD_UINT64;
                mv->mvDataLength = sizeof(unsigned long long);
                mv->mvData = (char *) mv + sizeof(MetricValue);
                *(unsigned long long *) mv->mvData = htonll(domain_statistics.cpu_ready[i]);
                mv->mvResource = (char *) mv + sizeof(MetricValue) + sizeof(unsigned long long);
                strcpy(mv->mvResource, domain_statistics.domain_name[i]);
                mret(mv);
            }
        }

        return 1;
    }

    return -1;
}

int virtMetricRetrVirtualBlockIOStats(int mid, MetricReturner mret)
{
    MetricValue *mv = NULL;
    struct vdisk_type * disk;
    char values[25*3+4];
    char * resource;

#ifdef DEBUG
    fprintf(stderr,
            "--- %s(%i) : Retrieving VirtualBlockIOStats\n",
            __FILE__, __LINE__);
#endif

    if (collectDomainStats() == VIRT_FAIL)
        return -1;

    if (mret == NULL) {
#ifdef DEBUG
        fprintf(stderr, 
                "--- %s(%i) : Returner pointer is NULL\n", 
                __FILE__, __LINE__);
#endif
    } else {
#ifdef DEBUG
        fprintf(stderr,
                "--- %s(%i) : Sampling for VirtualBlockIOStats\n",
                __FILE__, __LINE__);
#endif

        size_t i;

#ifdef DEBUG
        fprintf(stderr,
                "--- %s(%i) : num_active_domains %d\n",
                __FILE__, __LINE__, node_statistics.num_active_domains);
#endif

        for (i = 0; i < node_statistics.total_domains; i++) {
        
            disk = domain_statistics.blkio[i];

            while (disk) {
            
                memset(values,0,sizeof(values));
	            sprintf(values,"%lld:%lld:%lld:", disk->read, disk->write, disk->capacity);
	            
	            resource = malloc(strlen(domain_statistics.domain_name[i]) +
	                                strlen(disk->source) + strlen(disk->target) + 3);
	            sprintf(resource,"%s:%s:%s", domain_statistics.domain_name[i], disk->source, disk->target);

                mv = calloc(1, sizeof(MetricValue) +
                        (strlen(values) + 1) +
                        (strlen(resource) + 1));

                if (mv) {
                    mv->mvId = mid;
                    mv->mvTimeStamp = time(NULL);
                    mv->mvDataType = MD_STRING;
                    mv->mvDataLength = strlen(values) + 1;
                    mv->mvData = (char *) mv + sizeof(MetricValue);
                    strcpy(mv->mvData, values);
                    mv->mvResource = (char *) mv + sizeof(MetricValue) + (strlen(values) + 1);
                    strcpy(mv->mvResource, resource);
                    mret(mv);
                }
                
                disk = disk->next;
            }
        }

        return 1;
    }

    return -1;
}
