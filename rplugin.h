/*
 * $Id: rplugin.h,v 1.1 2004/07/16 15:30:05 mihajlov Exp $
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
 * Description: Repository Plugin Definitions
 *
 */

#ifndef RPLUGIN_H
#define RPLUGIN_H

#include "metric.h"

#define REPOSITORYMETRIC_DEFINITIONPROC _DefinedRepositoryMetrics

#define REPOSITORYMETRIC_DEFINITIONPROC_S "_DefinedRepositoryMetrics"

/* provided by caller */
typedef int (MetricRegisterId) (const char * pluginname, const char *midstr);

/* provided by plugin */
typedef int (RepositoryMetricsDefined) (MetricRegisterId *mr,
					const char * pluginname,
					size_t *mdnum,
					MetricCalculationDefinition **mc); 


#endif
