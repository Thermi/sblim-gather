/*
 * $Id: rreg.h,v 1.1 2004/07/16 15:30:05 mihajlov Exp $
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
 * Description: Repository Metric Registration Interface
 *
 */

#ifndef RREG_H
#define RREG_H

#include "metric.h"

#ifdef __cplusplus
extern "C" {
#endif
/* returns unique (process lifetime) metric id for given name */
int RPR_IdForString(const char *pluginname, const char *name);

/* initialize metric processing - optional */
void RPR_InitRegistry();

/* terminate metric processing - removes all metric definitions */
void RPR_FinishRegistry();

/* add new metric definition to registry */
int  RPR_UpdateMetric(const char *pluginname, MetricCalculationDefinition *metric);

/* returns metric definition for id  - this call is efficient */
MetricCalculationDefinition* RPR_GetMetric(int id);

/* delete metric definition for id */
void RPR_RemoveMetric(int id);
 
#ifdef __cplusplus
}
#endif

#endif
