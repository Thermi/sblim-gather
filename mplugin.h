/*
 * $Id: mplugin.h,v 1.1 2003/10/17 13:56:01 mihajlov Exp $
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
 * Description: Metric Plugin Definitions
 *
 */

#ifndef MPLUGIN_H
#define MPLUGIN_H

#include "metric.h"

#define METRIC_DEFINITIONPROC _DefinedMetrics
#define MTERIC_STARTSTOPPROC  _StartStopMetrics 

#define METRIC_DEFINITIONPROC_S "_DefinedMetrics"
#define METRIC_STARTSTOPPROC_S  "_StartStopMetrics"

/* provided by caller */
typedef int (MetricRegisterId) (const char * pluginname, const char *midstr);

/* provided by plugin */
typedef int (MetricsDefined) (MetricRegisterId *mr,
			      const char * pluginname,
			      size_t *mdnum,
			      MetricDefinition **md); 
typedef int (MetricStartStop) (int starting); 


#endif
