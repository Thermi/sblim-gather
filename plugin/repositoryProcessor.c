/*
 * $Id: repositoryProcessor.c,v 1.3 2004/12/03 15:57:22 mihajlov Exp $
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
 * Author:       Heidi Neumann <heidineu@de.ibm.com>
 * Contributors: 
 *
 * Description:
 * Repository Plugin of the following Processor specific metrics :
 *
 * TotalCPUTimePercentage
 *
 */

/* ---------------------------------------------------------------------------*/

#include <mplugin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commutil.h>

/* ---------------------------------------------------------------------------*/

static MetricCalculationDefinition metricCalcDef[1];

/* --- TotalCPUTimePercentage --- */
static MetricCalculator  metricCalcCPUTimePerc;

/* ---------------------------------------------------------------------------*/

int _DefinedRepositoryMetrics( MetricRegisterId *mr,
			       const char *pluginname,
			       size_t *mcnum,
			       MetricCalculationDefinition **mc ) {
#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Retrieving metric calculation definitions\n",
	  __FILE__,__LINE__);
#endif
  if (mr==NULL||mcnum==NULL||mc==NULL) {
    fprintf(stderr,"--- %s(%i) : invalid parameter list\n",__FILE__,__LINE__);
    return -1;
  }

  metricCalcDef[0].mcVersion=MD_VERSION;
  metricCalcDef[0].mcName="TotalCPUTimePercentage";
  metricCalcDef[0].mcId=mr(pluginname,metricCalcDef[0].mcName);
  metricCalcDef[0].mcMetricType=MD_PERIODIC|MD_RETRIEVED|MD_AVERAGE;
  metricCalcDef[0].mcChangeType=MD_GAUGE;
  metricCalcDef[0].mcIsContinuous=MD_TRUE;
  metricCalcDef[0].mcDataType=MD_FLOAT32;
  metricCalcDef[0].mcCalc=metricCalcCPUTimePerc;

  *mcnum=1;
  *mc=metricCalcDef;
  return 0;
}


/* ---------------------------------------------------------------------------*/
/* TotalCPUTimePercentage                                                     */
/* ---------------------------------------------------------------------------*/

size_t metricCalcCPUTimePerc( MetricValue *mv,   
			      int mnum,
			      void *v, 
			      size_t vlen ) {
  float total = 0;
  float sum   = 0;
  int i     = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate TotalCPUTimePercentage\n",
	  __FILE__,__LINE__);
#endif
  if ( mv && (vlen>=mv->mvDataLength)) {
    for(;i<mnum;i++) { sum = sum + ntohf(*(float*)mv->mvData); }
    total = sum / mnum;
    memcpy(v, &total, sizeof(float));
    return sizeof(float);
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/
/*                         end of repositoryProcessor.c                       */
/* ---------------------------------------------------------------------------*/
