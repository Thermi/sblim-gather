/*
 * $Id: repos.h,v 1.9 2004/11/10 16:08:24 heidineu Exp $
 *
 * (C) Copyright IBM Corp. 2004
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
 * Description: Local Repository Interface
 */

#ifndef REPOS_H
#define REPOS_H

#include <stdlib.h>
#include "commheap.h"

/* for quick test purposes use metric.h directly */
#include "metric.h"
#include "mrepos.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _RepositoryToken {
  unsigned long rt_size;
  unsigned long rt1;
  unsigned long rt2;
} RepositoryToken;

int repos_token(RepositoryToken *rt);

int repos_init(); 
int repos_terminate(); 

int repos_sessiontoken(RepositoryToken *rt);

int reposplugin_add(const char *pluginname);
int reposplugin_remove(const char *pluginname);

typedef struct _RepositoryPluginDefinition {
  int      rdId;
  unsigned rdDataType;
  unsigned rdMetricType;
  unsigned rdChangeType;
  unsigned char rdIsContinuous;
  char    *rdName; 
  char   **rdResource; 
} RepositoryPluginDefinition;

int reposplugin_list(const char *pluginname,
		     RepositoryPluginDefinition **pdef, 
		     COMMHEAP ch);

typedef struct _RepositoryStatus {
  short    rsInitialized;
  unsigned rsNumPlugins;
  unsigned rsNumMetrics;
} RepositoryStatus;

void repos_status(RepositoryStatus *rs); 

int reposresource_list(const char * metricid,
		       MetricResourceId **rid,
		       COMMHEAP ch);

typedef struct _ValueItem {
  time_t viCaptureTime;
  time_t viDuration;
  size_t viValueLen;
  char * viValue;
  char * viResource;
  char * viSystemId;
} ValueItem;

typedef struct _ValueRequest {
  int         vsId;
  char       *vsResource;
  char       *vsSystemId;
  time_t      vsFrom;
  time_t      vsTo;
  unsigned    vsDataType;
  int         vsNumValues;
  ValueItem  *vsValues;
} ValueRequest;

typedef void * MVENUM;

int reposvalue_put(const char *reposplugin, const char *metric, 
		   MetricValue *mv);
int reposvalue_get(ValueRequest *vs, COMMHEAP ch);

/* subscriptions --- */

#define SUBSCR_OP_ANY 0
#define SUBSCR_OP_EQ  1
#define SUBSCR_OP_NE  2
#define SUBSCR_OP_GT  3
#define SUBSCR_OP_GE  4
#define SUBSCR_OP_LT  5
#define SUBSCR_OP_LE  6

typedef struct _SubscriptionRequest {
  int    srCorrelatorId;
  int    srMetricId; /* required -- anything below can be SUBSCR_OP_ANY */
  int    srBaseMetricId; /* if the metric is a derived one this must be set */
  char  *srResource;
  int    srResourceOp;
  char  *srValue;
  int    srValueOp;
  char  *srSystemId;
  int    srSystemOp;
} SubscriptionRequest;

typedef void (SubscriptionCallback) (int corrid, ValueRequest *vr);

int repos_subscribe(SubscriptionRequest *sr, SubscriptionCallback *mcb);

#ifdef __cplusplus
}
#endif

#endif
