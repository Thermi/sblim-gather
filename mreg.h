/*
 * $Id: mreg.h,v 1.1 2003/10/17 13:56:01 mihajlov Exp $
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
 * Description: Metric Registration Interface
 *
 */

#ifndef MREG_H
#define MREG_H

#include "metric.h"

#ifdef __cplusplus
extern "C" {
#endif
/* returns unique (process lifetime) metric id for given name */
int MPR_IdForString(const char *pluginname, const char *name);

/* initialize metric processing - optional */
void MPR_InitRegistry();

/* terminate metric processing - removes all metric definitions */
void MPR_FinishRegistry();

/* add new metric definition to registry */
int  MPR_UpdateMetric(const char *pluginname, MetricDefinition *metric);

/* returns metric definition for id  - this call is efficient */
MetricDefinition* MPR_GetMetric(int id);

/* delete metric definition for id */
void MPR_RemoveMetric(int id);
 
#ifdef __cplusplus
}
#endif

#endif
