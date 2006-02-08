/*
 * $Id: gather.h,v 1.6 2006/02/08 20:26:45 mihajlov Exp $
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

int metricplugin_loadall();
int metricplugin_add(const char *pluginname);
int metricplugin_remove(const char *pluginname);

typedef struct _PluginDefinition {
  int      pdId;
  char    *pdName; 
} PluginDefinition;

int metricplugin_list(const char *pluginname,
		      PluginDefinition **pdef, 
		      COMMHEAP ch);


int metricpluginname_list(char ***pluginname,
			  COMMHEAP ch);

typedef struct _GatherStatus {
  short    gsInitialized;
  short    gsSampling;
  unsigned gsNumPlugins;
  unsigned gsNumMetrics;
} GatherStatus;

void gather_status(GatherStatus *gs); 

#ifdef __cplusplus
}
#endif

#endif
