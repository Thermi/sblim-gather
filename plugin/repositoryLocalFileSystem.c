/*
 * $Id: repositoryLocalFileSystem.c,v 1.3 2004/11/03 08:16:36 heidineu Exp $
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
 * Repository Plugin of the following file system specific metrics :
 *
 * AvailableSpace
 * AvailableSpacePercentage
 *
 */

/* ---------------------------------------------------------------------------*/

#include <mplugin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------------------------------------------------------------------------*/

static MetricCalculationDefinition metricCalcDef[2];

/* --- AvailableSpace --- */
static MetricCalculator  metricCalcAvSpace;

/* --- AvailableSpacePercentage --- */
static MetricCalculator  metricCalcAvSpacePerc;

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
  metricCalcDef[0].mcName="AvailableSpace";
  metricCalcDef[0].mcId=mr(pluginname,metricCalcDef[0].mcName);
  metricCalcDef[0].mcMetricType=MD_PERIODIC|MD_RETRIEVED|MD_POINT;
  metricCalcDef[0].mcChangeType=MD_GAUGE;
  metricCalcDef[0].mcIsContinuous=MD_TRUE;
  metricCalcDef[0].mcDataType=MD_UINT64;
  metricCalcDef[0].mcCalc=metricCalcAvSpace;

  metricCalcDef[1].mcVersion=MD_VERSION;
  metricCalcDef[1].mcName="AvailableSpacePercentage";
  metricCalcDef[1].mcId=mr(pluginname,metricCalcDef[1].mcName);
  metricCalcDef[1].mcMetricType=MD_PERIODIC|MD_RETRIEVED|MD_POINT;
  metricCalcDef[1].mcChangeType=MD_GAUGE;
  metricCalcDef[1].mcIsContinuous=MD_TRUE;
  metricCalcDef[1].mcDataType=MD_UINT8;
  metricCalcDef[1].mcCalc=metricCalcAvSpacePerc;

  *mcnum=2;
  *mc=metricCalcDef;
  return 0;
}


/* ---------------------------------------------------------------------------*/
/* AvailableSpace                                                             */
/* ---------------------------------------------------------------------------*/

size_t metricCalcAvSpace( MetricValue *mv,   
			  int mnum,
			  void *v, 
			  size_t vlen ) {

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate AvailableSpace\n",
	  __FILE__,__LINE__);
#endif
  /* plain copy */
  if (mv && (vlen>=mv->mvDataLength) && (mnum==1) ) {
    memcpy(v,mv->mvData,mv->mvDataLength);
    return mv->mvDataLength;
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/
/* AvailableSpacePercentage                                                   */
/* ---------------------------------------------------------------------------*/

size_t metricCalcAvSpacePerc( MetricValue *mv,   
			      int mnum,
			      void *v, 
			      size_t vlen ) {

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate AvailableSpacePercentage\n",
	  __FILE__,__LINE__);
#endif
  /* plain copy */
  if (mv && (vlen>=mv->mvDataLength) && (mnum==1) ) {
    memcpy(v,mv->mvData,mv->mvDataLength);
    return mv->mvDataLength;
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/
/*                  end of repositoryLocalFileSystem.c                        */
/* ---------------------------------------------------------------------------*/
