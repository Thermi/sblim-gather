#ifndef METRICVIRT_H
#define METRICVIRT_H

#include <mplugin.h>

#define XEN_HYP 0
#define KVM_HYP 1

#define MAX_DOMAINS 255

struct node_statistics_type {
	unsigned int num_domains;
	unsigned long long total_memory;
	unsigned long long free_memory;
} node_statistics;

struct domain_statistics_type {
	unsigned int domain_id[MAX_DOMAINS];
	char * domain_name[MAX_DOMAINS];
	unsigned long long claimed_memory[MAX_DOMAINS];
	unsigned long long max_memory[MAX_DOMAINS];
	float cpu_time[MAX_DOMAINS];
	unsigned short vcpus[MAX_DOMAINS];
} domain_statistics;

int connectHypervisor(int type);

int collectNodeStats();
int collectDomainStats();

MetricRetriever virtMetricRetrCPUTime;
MetricRetriever virtMetricRetrTotalCPUTime;
MetricRetriever virtMetricRetrActiveVirtualProcessors;
MetricRetriever virtMetricRetrInternalMemory;
MetricRetriever virtMetricRetrHostFreePhysicalMemory;

#endif
