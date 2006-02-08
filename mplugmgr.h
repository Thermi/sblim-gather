/*
 * $Id: mplugmgr.h,v 1.2 2006/02/08 20:26:46 mihajlov Exp $
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
 * Description: Metric Plugin Manager Interface
 */

#ifndef MPLUGMGR_H
#define MPLUGMGR_H

#include "metric.h"
#include "mplugin.h"

/* plugin structure */
typedef struct _MetricPlugin {
  /* supplied by caller */
  char             *mpName;
  MetricRegisterId *mpRegister;
  /* supplied by Load */
  void             *mpHandle;
  size_t            mpNumMetricDefs;
  MetricDefinition *mpMetricDefs;
} MetricPlugin;

int MP_SetPluginPath(const char *pluginpath);
char* MP_GetPluginPath();
int MP_Load(MetricPlugin *plugin);
int MP_Unload(MetricPlugin *plugin);

#endif
