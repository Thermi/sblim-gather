/*
 * $Id: gather.h,v 1.1 2003/10/17 13:56:01 mihajlov Exp $
 *
 * (C) Copyright IBM Corp. 2003
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
 * Description: Local Gatherer Interface
 */

#ifndef GATHER_H
#define GATHER_H

#include <stdlib.h>
#include "commheap.h"

#ifdef __cplusplus
extern "C" {
#endif

int gather_init(); 
int gather_start(); 
int gather_stop(); 
int gather_terminate(); 

int metricplugin_add(const char *pluginname);
int metricplugin_remove(const char *pluginname);

typedef struct _PluginDefinition {
  int      pdId;
  unsigned pdDataType;
  char    *pdName; 
  char   **pdResource; 
} PluginDefinition;

int metricplugin_list(const char *pluginname,
		      PluginDefinition **pdef, 
		      COMMHEAP ch);

typedef struct _GatherStatus {
  short    gsInitialized;
  short    gsSampling;
  unsigned gsNumPlugins;
  unsigned gsNumMetrics;
} GatherStatus;

void gather_status(GatherStatus *gs); 

typedef struct _ValueItem {
  time_t viCaptureTime;
  time_t viDuration;
  size_t viValueLen;
  char * viValue;
  char * viResource;
} ValueItem;

typedef struct _ValueRequest {
  int         vsId;
  char       *vsResource;
  time_t      vsFrom;
  time_t      vsTo;
  unsigned    vsDataType;
  int         vsNumValues;
  ValueItem  *vsValues;
} ValueRequest;

typedef void * MVENUM;

int metricvalue_get(ValueRequest *vs, COMMHEAP ch);

#ifdef __cplusplus
}
#endif

#endif
