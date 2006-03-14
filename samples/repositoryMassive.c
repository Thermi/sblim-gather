/*
 * $Id: repositoryMassive.c,v 1.1 2006/03/14 08:25:22 mihajlov Exp $
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
 * Description: Sample Code
 */

#include <mplugin.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

static MetricCalculationDefinition metricCalcDef;

static MetricCalculator  metricCalculator;

int _DefinedRepositoryMetrics (MetricRegisterId *mr,
			       const char *pluginname,
			       size_t *mcnum,
			       MetricCalculationDefinition **mc)
{
#ifdef DEBUG  
  fprintf(stderr,"Retrieving metric definitions\n");
#endif
  if (mr==NULL||mcnum==NULL||mc==NULL) {
    fprintf(stderr,"invalid parameter list\n");
    return -1;
  }
  metricCalcDef.mcVersion=MD_VERSION;
  metricCalcDef.mcName="massive";
  metricCalcDef.mcId=mr(pluginname,metricCalcDef.mcName);
  metricCalcDef.mcMetricType=MD_RETRIEVED | MD_POINT;
  metricCalcDef.mcDataType=MD_UINT32;
  metricCalcDef.mcCalc=metricCalculator;
  *mcnum=1;
  *mc=&metricCalcDef;
  return 0;
}

size_t metricCalculator(MetricValue *mv, int mnum, void *v, size_t vlen)
{
  /* plain copy */
  if (mv && vlen >=  mv->mvDataLength) {
    *(time_t*)v=ntohl(*(time_t*)mv->mvData);
    return mv->mvDataLength;
  }
  return -1;
}

