/*
 * $Id: rplugmgr.h,v 1.1 2004/07/16 15:30:05 mihajlov Exp $
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
 * Description: Repository Plugin Manager Interface
 */

#ifndef RPLUGMGR_H
#define RPLUGMGR_H

#include "metric.h"
#include "rplugin.h"

/* plugin structure */
typedef struct _RepositoryPlugin {
  /* supplied by caller */
  char                        *rpName;
  MetricRegisterId            *rpRegister;
  /* supplied by plugin */
  void                        *rpHandle;
  size_t                       rpNumMetricCalcDefs;
  MetricCalculationDefinition *rpMetricCalcDefs;
} RepositoryPlugin;

int RP_Load(RepositoryPlugin *plugin);
int RP_Unload(RepositoryPlugin *plugin);

#endif
