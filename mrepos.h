/*
 * $Id: mrepos.h,v 1.6 2004/12/02 17:46:49 mihajlov Exp $
 *
 * (C) Copyright IBM Corp. 2003, 2004
 *
 * THIS FILE IS PROVIDED UNDER THE TERMS OF THE COMMON PUBLIC LICENSE
 * ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
 * CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
 *
 * You can obtain a current copy of the Common Public License from
 * http://oss.software.ibm.com/developerworks/opensource/license-cpl.html
 *
 * Author:       Viktor Mihajlovski <mihajlov@de.ibm.cim>
 * Contributors: 
 *
 * Description: Metric Repository Definitions
 *
 * The MetricRepositoryIF structure holds a set of function pointers
 * implementing the repository interface functions.
 *
 * MetricAdd is used to store the raw data into the repository
 * MetricRetrieve is used to retrieve the raw data if available from the 
 * repository.
 * Those functions are for internal processing only and not meant to be used 
 * by consumers.
 *
 */

#ifndef MREPOS_H
#define MREPOS_H

#include "metric.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MCB_STATE_REGISTER   1
#define MCB_STATE_UNREGISTER 0

typedef struct _MetricResourceId {
  char * mrid_resource;
  char * mrid_system;
} MetricResourceId;

/* callback definition for subscription processing */
typedef void (MetricCallback) (MetricValue *mv, int num);

typedef int (MetricAdd) (MetricValue *mv);
typedef int (MetricRetrieve) (int mid, MetricResourceId *resource,
			      MetricValue **mv, int *num, 
			      time_t from, time_t to, int maxnum);
typedef int (MetricRelease) (MetricValue *mv);
typedef int (MetricRegisterCallback) (MetricCallback* mcb, int mid, int state);
typedef int (ResourcesRetrieve) (int mid, MetricResourceId **resources,
				 const char * resource, const char * system);
typedef int (ResourcesRelease) (int mid, MetricResourceId *resources);
  
typedef struct _MetricRepositoryIF {
  char                   *mrep_name;
  MetricAdd              *mrep_add;
  MetricRetrieve         *mrep_retrieve;
  MetricRelease          *mrep_release;
  MetricRegisterCallback *mrep_regcallback;
  ResourcesRetrieve      *mres_retrieve;
  ResourcesRelease       *mres_release;
} MetricRepositoryIF;

extern MetricRepositoryIF *MetricRepository;

#ifdef __cplusplus
}
#endif

#endif
