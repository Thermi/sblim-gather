/*
 * $Id: metricSample4.c,v 1.3 2004/08/02 14:23:02 mihajlov Exp $
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
  metricDef[0].mdVersion=MD_VERSION;
  metricDef[0].mdName="sample4-1";
  metricDef[0].mdReposPluginName="samples/librepositorySample.so";
  metricDef[0].mdId=mr(pluginname,metricDef[0].mdName);
  metricDef[0].mdSampleInterval=30;
  metricDef[0].mdMetricType=MD_RETRIEVED | MD_POINT;
  metricDef[0].mdDataType=MD_SINT32;
  metricDef[0].mproc=metricRetriever;
  metricDef[0].mdeal=free;
  metricDef[1].mdVersion=MD_VERSION;
  metricDef[1].mdName="sample4-2";
  metricDef[1].mdReposPluginName="samples/librepositorySample.so";
  metricDef[1].mdId=mr(pluginname,metricDef[1].mdName);
  metricDef[1].mdMetricType=MD_CALCULATED | MD_INTERVAL;
  metricDef[1].mdDataType=MD_SINT32;
  //  metricDef[1].mdAliasId=metricDef[0].mdId;
  metricDef[1].mproc=NULL;
  metricDef[1].mdeal=NULL;
  metricDef[2].mdVersion=MD_VERSION;
  metricDef[2].mdName="sample4-3";
  metricDef[2].mdReposPluginName="samples/librepositorySample.so";
  metricDef[2].mdId=mr(pluginname,metricDef[2].mdName);
  metricDef[2].mdMetricType=MD_CALCULATED | MD_RATE    ;
  metricDef[2].mdDataType=MD_SINT32;
  //  metricDef[2].mdAliasId=metricDef[0].mdId;
  metricDef[2].mproc=NULL;
  metricDef[2].mdeal=NULL;
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

