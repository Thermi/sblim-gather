/*
 * $Id: metricSample4.c,v 1.1 2003/10/17 13:56:01 mihajlov Exp $
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
 * Description: Sample Code
 */

#include <mplugin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static MetricDefinition metricDef[3];

static MetricRetriever   metricRetriever;
static MetricCalculator  metricCalculator1;
static MetricCalculator  metricCalculator2;
static MetricCalculator  metricCalculator3;
static ResourceLister    resourceLister;
static ResourceListDeallocator  resourceListDeallocator;

int _DefinedMetrics (MetricRegisterId *mr,
		     const char *pluginname,
		     size_t *mdnum,
		     MetricDefinition **md)
{
#ifdef DEBUG
  fprintf(stderr,"Retrieving metric definitions\n");
#endif
  if (mr==NULL||mdnum==NULL||md==NULL) {
    fprintf(stderr,"invalid parameter list\n");
    return -1;
  }
  metricDef[0].mdName="sample4-1";
  metricDef[0].mdId=mr(pluginname,metricDef[0].mdName);
  metricDef[0].mdSampleInterval=30;
  metricDef[0].mdMetricType=MD_RETRIEVED | MD_POINT;
  metricDef[0].mdDataType=MD_SINT32;
  metricDef[0].mproc=metricRetriever;
  metricDef[0].mdeal=free;
  metricDef[0].mcalc=metricCalculator1;
  metricDef[0].mresl=resourceLister;
  metricDef[0].mresldeal=resourceListDeallocator;
  metricDef[1].mdName="sample4-2";
  metricDef[1].mdId=mr(pluginname,metricDef[1].mdName);
  metricDef[1].mdMetricType=MD_CALCULATED | MD_INTERVAL;
  metricDef[1].mdDataType=MD_SINT32;
  metricDef[1].mdAliasId=metricDef[0].mdId;
  metricDef[1].mproc=NULL;
  metricDef[1].mdeal=NULL;
  metricDef[1].mresl=metricDef[0].mresl;
  metricDef[1].mresldeal=metricDef[0].mresldeal;
  metricDef[1].mcalc=metricCalculator2;
  metricDef[2].mdName="sample4-3";
  metricDef[2].mdId=mr(pluginname,metricDef[2].mdName);
  metricDef[2].mdMetricType=MD_CALCULATED | MD_RATE    ;
  metricDef[2].mdDataType=MD_SINT32;
  metricDef[2].mdAliasId=metricDef[0].mdId;
  metricDef[2].mproc=NULL;
  metricDef[2].mdeal=NULL;
  metricDef[2].mresl=metricDef[0].mresl;
  metricDef[2].mresldeal=metricDef[0].mresldeal;
  metricDef[2].mcalc=metricCalculator3;
  *mdnum=3;
  *md=metricDef;
  return 0;
}

int _StartStopMetrics (int starting) 
{
#ifdef DEBUG
  fprintf(stderr,"%s metric processing\n",starting?"Starting":"Stopping");
#endif
  return 0;
}

int metricRetriever(int mid, MetricReturner mret)
{
  int numvalues = 0;
  MetricValue *mv;

  if (mret==NULL)
    fprintf(stderr,"Returner pointer is NULL\n");
  else {
#ifdef DEBUG
    fprintf(stderr,"Sampling for metric id %d\n",mid);
#endif
    mv = malloc(sizeof(MetricValue) + sizeof(int));
    if (mv) {
      mv->mvId = mid;
      mv->mvResource = "sample4res";
      mv->mvTimeStamp = time(NULL);
      mv->mvDataLength = sizeof(int);
      mv->mvData = (void*)mv + sizeof(MetricValue);
      *(int*)mv->mvData = 42+mid;
      mret(mv);
      numvalues += 1;
    }
    mv = malloc(sizeof(MetricValue) + sizeof(int));
    if (mv) {
      mv->mvId = mid;
      mv->mvResource = "sample4res2";
      mv->mvTimeStamp = time(NULL);
      mv->mvDataLength = sizeof(int);
      mv->mvData = (void*)mv + sizeof(MetricValue);
      *(int*)mv->mvData = 4711+mid;
      mret(mv);
      numvalues += 1;
    }
  }
  return numvalues;
}

size_t metricCalculator1(MetricValue *mv, int mnum, void *v, size_t vlen)
{
  /* plain copy */
  if (mv && vlen >=  mv->mvDataLength) {
    memcpy(v,mv->mvData,mv->mvDataLength);
    return mv->mvDataLength;
  }
  return -1;
}

size_t metricCalculator2(MetricValue *mv, int mnum, void *v, size_t vlen)
{
  /* some calculation*/
  int i1, i2;
  if (mv && vlen >=  mv->mvDataLength) {
    i1 = *(int*)mv[0].mvData;
    if (mnum > 1) {
      i2 = *(int*)mv[mnum-1].mvData;
    } else {
      i2 = 0;
    }
    *(int*)v = i1 - i2;
    return sizeof(int);
  }
  return -1;
}

size_t metricCalculator3(MetricValue *mv, int mnum, void *v, size_t vlen)
{
  /* some calculation*/
  int i1, i2;
  if (mv && vlen >=  mv->mvDataLength &&
      strcmp(mv->mvResource,"sample4res") == 0) {
    i1 = *(int*)mv[0].mvData;
    if (mnum > 1) {
      i2 = *(int*)mv[mnum-1].mvData;
    } else {
      i2 = 0;
    }
    *(int*)v = i1 - i2;
    return sizeof(int);
  }
  return -1;
}

static char * resourcelist[] = { "sample4res",
				 "sample4res2"};

int resourceLister(int mid, char *** list)
{
  if (list) *list = resourcelist;
  return 2;
}

void resourceListDeallocator(char ** list)
{
}
