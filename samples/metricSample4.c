/*
 * $Id: metricSample4.c,v 1.4 2004/08/03 10:19:33 mihajlov Exp $
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
  metricDef[0].mdReposPluginName="samples/librepositorySample4.so";
  metricDef[0].mdId=mr(pluginname,metricDef[0].mdName);
  metricDef[0].mdSampleInterval=30;
  metricDef[0].mproc=metricRetriever;
  metricDef[0].mdeal=free;
  *mdnum=1;
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

