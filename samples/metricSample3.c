/*
 * $Id: metricSample3.c,v 1.1 2003/10/17 13:56:01 mihajlov Exp $
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

static MetricDefinition metricDef;

static MetricRetriever   metricRetriever;
static MetricDeallocator metricDeallocator;
static MetricCalculator  metricCalculator;

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
  metricDef.mdName="sample3";
  metricDef.mdId=mr(pluginname,metricDef.mdName);
  metricDef.mdSampleInterval=20;
  metricDef.mdMetricType=MD_RETRIEVED;
  metricDef.mproc=metricRetriever;
  metricDef.mdeal=metricDeallocator;
  metricDef.mcalc=metricCalculator;
  *mdnum=1;
  *md=&metricDef;
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
  static int  intbuffer;
  int         numvalue = 0;
  MetricValue *mv;
  if (mret==NULL)
    fprintf(stderr,"Returner pointer is NULL\n");
  else {
#ifdef DEBUG
    fprintf(stderr,"Sampling for metric id %d\n",mid);
#endif
    mv = malloc(sizeof(MetricValue) + sizeof(int));
    if (mv) {
      mv->mvId = metricDef.mdId;
      mv->mvResource = "sample3res";
      mv->mvTimeStamp = 0;
      mv->mvDataLength = sizeof(int);
      intbuffer = 42;
      mv->mvData = (char *)&intbuffer;
      numvalue = 1;
      mret(mv);
    }
  }
  return numvalue;
}

size_t metricCalculator(MetricValue *mv, int mnum, void *v, size_t vlen)
{
  /* plain copy */
  if (mv && vlen >=  mv->mvDataLength) {
    *(int*)v=*(int*)mv->mvData;
    return mv->mvDataLength;
  }
  return -1;
}

void metricDeallocator(void *v)
{
  free(v);
}
