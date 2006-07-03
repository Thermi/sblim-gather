/*
 * $Id: repositoryzCEC.c,v 1.1 2006/07/03 15:27:37 mihajlov Exp $
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
 * Author:       Viktor Mihajlovski <mihajlov@de.ibm.com>
 * Contributors: 
 *
 * Description:
 * Metrics Repository Plugin of the following CEC Metrics :
 *  IBM_zCEC.KernelModeTime
 *  IBM_zCEC.UserModeTime
 *  IBM_zCEC.TotalCPUTime
 *  IBM_zCEC.UnusedGlobalCPUCapacity
 *  IBM_zCEC.ExternalViewKernelModePercentage
 *  IBM_zCEC.ExternalViewUserModePercentage
 *  IBM_zCEC.ExternalViewTotalCPUPercentage
 *
 *
 */

/* ---------------------------------------------------------------------------*/

#include <mplugin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commutil.h>

#ifdef DEBUG
#define DBGONLY(X) X
#else
#define DBGONLY(X)
#endif

/* ---------------------------------------------------------------------------*/

/* metric definition block */
static MetricCalculationDefinition metricCalcDef[8];

/* metric calculator callbacks */
static MetricCalculator  metricCalc_CECTimes;
static MetricCalculator  metricCalcCECKernelModeTime;
static MetricCalculator  metricCalcCECUserModeTime;
static MetricCalculator  metricCalcCECTotalCPUTime;
static MetricCalculator  metricCalcCECUnusedGlobalCPUCapacity;
static MetricCalculator  metricCalcCECExternalViewKernelModePercentage;
static MetricCalculator  metricCalcCECExternalViewUserModePercentage;
static MetricCalculator  metricCalcCECExternalViewTotalCPUPercentage;

/* unit definitions */
static char * muNA = "n/a";
static char * muMilliSeconds = "Milliseconds";
static char * muPercent = "Percent";

/* ---------------------------------------------------------------------------*/

int _DefinedRepositoryMetrics( MetricRegisterId *mr,
			       const char *pluginname,
			       size_t *mcnum,
			       MetricCalculationDefinition **mc ) {
  DBGONLY(fprintf(stderr,"--- %s(%i) : Retrieving metric calculation definitions\n", __FILE__,__LINE__);)

  if (mr==NULL||mcnum==NULL||mc==NULL) {
    fprintf(stderr,"--- %s(%i) : invalid parameter list\n",__FILE__,__LINE__);
    return -1;
  }

  metricCalcDef[0].mcVersion=MD_VERSION;
  metricCalcDef[0].mcName="_CECTimes";
  metricCalcDef[0].mcId=mr(pluginname,metricCalcDef[0].mcName);
  metricCalcDef[0].mcMetricType=MD_PERIODIC|MD_RETRIEVED|MD_POINT;
  metricCalcDef[0].mcChangeType=MD_GAUGE;
  metricCalcDef[0].mcIsContinuous=MD_TRUE;
  metricCalcDef[0].mcCalculable=MD_SUMMABLE;
  metricCalcDef[0].mcDataType=MD_STRING;
  metricCalcDef[0].mcCalc=metricCalc_CECTimes;
  metricCalcDef[0].mcUnits=muNA;

  metricCalcDef[1].mcVersion=MD_VERSION;
  metricCalcDef[1].mcName="KernelModeTime";
  metricCalcDef[1].mcId=mr(pluginname,metricCalcDef[1].mcName);
  metricCalcDef[1].mcMetricType=MD_PERIODIC|MD_CALCULATED|MD_INTERVAL;
  metricCalcDef[1].mcChangeType=MD_GAUGE;
  metricCalcDef[1].mcIsContinuous=MD_TRUE;
  metricCalcDef[1].mcCalculable=MD_SUMMABLE;
  metricCalcDef[1].mcDataType=MD_UINT64;
  metricCalcDef[1].mcAliasId=metricCalcDef[0].mcId;
  metricCalcDef[1].mcCalc=metricCalcCECKernelModeTime;
  metricCalcDef[1].mcUnits=muMilliSeconds;

  metricCalcDef[2].mcVersion=MD_VERSION;
  metricCalcDef[2].mcName="UserModeTime";
  metricCalcDef[2].mcId=mr(pluginname,metricCalcDef[2].mcName);
  metricCalcDef[2].mcMetricType=MD_PERIODIC|MD_CALCULATED|MD_INTERVAL;
  metricCalcDef[2].mcChangeType=MD_GAUGE;
  metricCalcDef[2].mcIsContinuous=MD_TRUE;
  metricCalcDef[2].mcCalculable=MD_SUMMABLE;
  metricCalcDef[2].mcDataType=MD_UINT64;
  metricCalcDef[2].mcAliasId=metricCalcDef[0].mcId;
  metricCalcDef[2].mcCalc=metricCalcCECUserModeTime;
  metricCalcDef[2].mcUnits=muMilliSeconds;

  metricCalcDef[3].mcVersion=MD_VERSION;
  metricCalcDef[3].mcName="TotalCPUTime";
  metricCalcDef[3].mcId=mr(pluginname,metricCalcDef[3].mcName);
  metricCalcDef[3].mcMetricType=MD_PERIODIC|MD_CALCULATED|MD_INTERVAL;
  metricCalcDef[3].mcChangeType=MD_GAUGE;
  metricCalcDef[3].mcIsContinuous=MD_TRUE;
  metricCalcDef[3].mcCalculable=MD_SUMMABLE;
  metricCalcDef[3].mcDataType=MD_UINT64;
  metricCalcDef[3].mcAliasId=metricCalcDef[0].mcId;
  metricCalcDef[3].mcCalc=metricCalcCECTotalCPUTime;
  metricCalcDef[3].mcUnits=muMilliSeconds;

  metricCalcDef[4].mcVersion=MD_VERSION;
  metricCalcDef[4].mcName="UnusedGlobalCPUCapacity";
  metricCalcDef[4].mcId=mr(pluginname,metricCalcDef[4].mcName);
  metricCalcDef[4].mcMetricType=MD_PERIODIC|MD_CALCULATED|MD_INTERVAL;
  metricCalcDef[4].mcChangeType=MD_GAUGE;
  metricCalcDef[4].mcIsContinuous=MD_TRUE;
  metricCalcDef[4].mcCalculable=MD_SUMMABLE;
  metricCalcDef[4].mcDataType=MD_UINT64;
  metricCalcDef[4].mcAliasId=metricCalcDef[0].mcId;
  metricCalcDef[4].mcCalc=metricCalcCECUnusedGlobalCPUCapacity;
  metricCalcDef[4].mcUnits=muMilliSeconds;

  metricCalcDef[5].mcVersion=MD_VERSION;
  metricCalcDef[5].mcName="ExternalViewKernelModePercentage";
  metricCalcDef[5].mcId=mr(pluginname,metricCalcDef[5].mcName);
  metricCalcDef[5].mcMetricType=MD_PERIODIC|MD_CALCULATED|MD_INTERVAL;
  metricCalcDef[5].mcChangeType=MD_GAUGE;
  metricCalcDef[5].mcIsContinuous=MD_TRUE;
  metricCalcDef[5].mcCalculable=MD_NONSUMMABLE;
  metricCalcDef[5].mcDataType=MD_FLOAT32;
  metricCalcDef[5].mcAliasId=metricCalcDef[0].mcId;
  metricCalcDef[5].mcCalc=metricCalcCECExternalViewKernelModePercentage;
  metricCalcDef[5].mcUnits=muPercent;
 
  metricCalcDef[6].mcVersion=MD_VERSION;
  metricCalcDef[6].mcName="ExternalViewUserModePercentage";
  metricCalcDef[6].mcId=mr(pluginname,metricCalcDef[6].mcName);
  metricCalcDef[6].mcMetricType=MD_PERIODIC|MD_CALCULATED|MD_INTERVAL;
  metricCalcDef[6].mcChangeType=MD_GAUGE;
  metricCalcDef[6].mcIsContinuous=MD_TRUE;
  metricCalcDef[6].mcCalculable=MD_NONSUMMABLE;
  metricCalcDef[6].mcDataType=MD_FLOAT32;
  metricCalcDef[6].mcAliasId=metricCalcDef[0].mcId;
  metricCalcDef[6].mcCalc=metricCalcCECExternalViewUserModePercentage;
  metricCalcDef[6].mcUnits=muPercent;
 
  metricCalcDef[7].mcVersion=MD_VERSION;
  metricCalcDef[7].mcName="ExternalViewTotalCPUPercentage";
  metricCalcDef[7].mcId=mr(pluginname,metricCalcDef[7].mcName);
  metricCalcDef[7].mcMetricType=MD_PERIODIC|MD_CALCULATED|MD_INTERVAL;
  metricCalcDef[7].mcChangeType=MD_GAUGE;
  metricCalcDef[7].mcIsContinuous=MD_TRUE;
  metricCalcDef[7].mcCalculable=MD_NONSUMMABLE;
  metricCalcDef[7].mcDataType=MD_FLOAT32;
  metricCalcDef[7].mcAliasId=metricCalcDef[0].mcId;
  metricCalcDef[7].mcCalc=metricCalcCECExternalViewTotalCPUPercentage;
  metricCalcDef[7].mcUnits=muPercent;
 
  *mcnum=sizeof(metricCalcDef)/sizeof(MetricCalculationDefinition);
  *mc=metricCalcDef;
  return 0;
}


/* ---------------------------------------------------------------------------*/
/* _CECTimes (compound raw metric)                                            */
/* ---------------------------------------------------------------------------*/

size_t metricCalc_CECTimes( MetricValue *mv,   
			    int mnum,
			    void *v, 
			    size_t vlen ) 
{
  DBGONLY(fprintf(stderr,"--- %s(%i) : Calculate CEC._CECTimes\n", __FILE__,__LINE__);)

  if ( mv && (vlen>=strlen(mv->mvData))) {
    strcpy(v,mv->mvData);
    return strlen(mv->mvData);
  }
  return -1;
}

/* ---------------------------------------------------------------------------*/
/* Kernel Mode Time                                                           */
/* ---------------------------------------------------------------------------*/

size_t metricCalcCECKernelModeTime( MetricValue *mv,   
				    int mnum,
				    void *v, 
				    size_t vlen ) 
{
  unsigned long long kmt1, kmt2;
  DBGONLY(fprintf(stderr,"--- %s(%i) : Calculate CEC.KernelModeTime\n", __FILE__,__LINE__);)

  if ( mv && (vlen>=sizeof(unsigned long long))) {
    sscanf(mv[0].mvData,"%llu:%*d:%*u",&kmt1);
    sscanf(mv[mnum-1].mvData,"%llu:%*d:%*u",&kmt2);
    *(unsigned long long *)v = (kmt1 - kmt2) / 1000;
    return sizeof(unsigned long long);
  }
  return -1;
}

/* ---------------------------------------------------------------------------*/
/* User Mode Time                                                           */
/* ---------------------------------------------------------------------------*/

size_t metricCalcCECUserModeTime( MetricValue *mv,   
				  int mnum,
				  void *v, 
				  size_t vlen ) 
{
  unsigned long long umt1, umt2;
  DBGONLY(fprintf(stderr,"--- %s(%i) : Calculate CEC.UserModeTime\n", __FILE__,__LINE__);)

  if ( mv && (vlen>=sizeof(unsigned long long))) {
    sscanf(mv[0].mvData,"%*u:%*d:%llu",&umt1);
    sscanf(mv[mnum-1].mvData,"%*u:%*d:%llu",&umt2);
    *(unsigned long long *)v = (umt1 - umt2) / 1000;
    return sizeof(unsigned long long);
  }
  return -1;
}

/* ---------------------------------------------------------------------------*/
/* Total CPU Time                                                           */
/* ---------------------------------------------------------------------------*/

size_t metricCalcCECTotalCPUTime( MetricValue *mv,   
				  int mnum,
				  void *v, 
				  size_t vlen ) 
{
  unsigned long long umt1, umt2;
  unsigned long long kmt1, kmt2;
  DBGONLY(fprintf(stderr,"--- %s(%i) : Calculate CEC.TotalCPUTime\n", __FILE__,__LINE__);)

  if ( mv && (vlen>=sizeof(unsigned long long))) {
    sscanf(mv[0].mvData,"%llu:%*d:%llu",&kmt1,&umt1);
    sscanf(mv[mnum-1].mvData,"%llu:%*d:%llu",&kmt2,&umt2);
    *(unsigned long long *)v = ((kmt1 + umt1) - (kmt2 + umt2))/1000;
    return sizeof(unsigned long long);
  }
  return -1;
}

/* ---------------------------------------------------------------------------*/
/* Unused Global CPU Capacity                                                 */
/* ---------------------------------------------------------------------------*/

size_t metricCalcCECUnusedGlobalCPUCapacity( MetricValue *mv,   
					     int mnum,
					     void *v, 
					     size_t vlen ) 
{
  unsigned long long umt1, umt2;
  unsigned long long kmt1, kmt2;
  int                num_cpus;
  time_t             interval;

  DBGONLY(fprintf(stderr,"--- %s(%i) : Calculate CEC.UnusedGlobalCPUCapacity\n", __FILE__,__LINE__);)

  if ( mv && (vlen>=sizeof(unsigned long long))) {
    sscanf(mv[0].mvData,"%llu:%d:%llu",&kmt1,&num_cpus,&umt1);
    sscanf(mv[mnum-1].mvData,"%llu:%*d:%llu",&kmt2,&umt2);
    interval = mv[0].mvTimeStamp - mv[mnum-1].mvTimeStamp;
    if ( interval > 0) {
      *(unsigned long long *)v = interval*1000*num_cpus - ((kmt1 + umt1) - (kmt2 + umt2))/1000;
    } else {
      *(unsigned long long *)v = 0ULL;
    }
    return sizeof(unsigned long long);
  }
  return -1;
}

/* ---------------------------------------------------------------------------*/
/* ExternalViewKernelModePercentage                                           */
/* ---------------------------------------------------------------------------*/

size_t metricCalcCECExternalViewKernelModePercentage( MetricValue *mv,   
						      int mnum,
						      void *v, 
						      size_t vlen ) 
{
  unsigned long long kernel1;
  unsigned long long kernel2;
  int                num_cpus;
  time_t             interval;
  DBGONLY(fprintf(stderr,"--- %s(%i) : Calculate CEC.ExternalViewKernelModePercentage\n", __FILE__,__LINE__);)

  if ( mv && (vlen>=sizeof(float))) {
    sscanf(mv[0].mvData,"%llu:%*d:%*u",&kernel1);
    sscanf(mv[mnum-1].mvData,"%llu:%d:%*u",&kernel2,&num_cpus);
    interval = mv[0].mvTimeStamp - mv[mnum-1].mvTimeStamp;
    if ( interval > 0) {
      /* raw values are microseconds */
      *(float*) v = (float)(kernel1 - kernel2)/(interval*10000*num_cpus);
      return sizeof(float);
    }
  }
  return -1;
}

/* ---------------------------------------------------------------------------*/
/* ExternalViewUserModePercentage                                             */
/* ---------------------------------------------------------------------------*/

size_t metricCalcCECExternalViewUserModePercentage( MetricValue *mv,   
						    int mnum,
						    void *v, 
						    size_t vlen ) 
{
  unsigned long long user1;
  unsigned long long user2;
  int                num_cpus;
  time_t             interval;
  DBGONLY(fprintf(stderr,"--- %s(%i) : Calculate CEC.ExternalViewUserModePercentage\n", __FILE__,__LINE__);)

  if ( mv && (vlen>=sizeof(float))) {
    sscanf(mv[0].mvData,"%*u:%*d:%llu",&user1);
    sscanf(mv[mnum-1].mvData,"%*u:%d:%llu",&num_cpus,&user2);
    interval = mv[0].mvTimeStamp - mv[mnum-1].mvTimeStamp;
    if ( interval > 0) {
      /* raw values are microseconds */
      *(float*) v = (float)(user1 - user2)/(interval*10000*num_cpus);
      return sizeof(float);
    }
  }
  return -1;
}

/* ---------------------------------------------------------------------------*/
/* ExternalViewTotalCPUPercentage                                           */
/* ---------------------------------------------------------------------------*/

size_t metricCalcCECExternalViewTotalCPUPercentage( MetricValue *mv,   
						    int mnum,
						    void *v, 
						    size_t vlen ) 
{
  unsigned long long kernel1, user1;
  unsigned long long kernel2, user2;
  int                num_cpus;
  time_t             interval;
  DBGONLY(fprintf(stderr,"--- %s(%i) : Calculate CEC.ExternalViewTotalCPUPercentage\n", __FILE__,__LINE__);)

  if ( mv && (vlen>=sizeof(float))) {
    sscanf(mv[0].mvData,"%llu:%*d:%llu",&kernel1,&user1);
    sscanf(mv[mnum-1].mvData,"%llu:%d:%llu",&kernel2,&num_cpus,&user2);
    interval = mv[0].mvTimeStamp - mv[mnum-1].mvTimeStamp;
    if ( interval > 0) {
      /* raw values are microseconds */
      *(float*) v = (float)((user1 + kernel1) - (user2 + kernel2))/(interval*10000*num_cpus);
      return sizeof(float);
    }
  }
  return -1;
}

