
/*
 * $Id: OSBase_MetricIndicationProvider.c,v 1.1 2004/12/13 14:00:53 mihajlov Exp $
 *
 * (C) Copyright IBM Corp. 2003, 2004
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
 * Interface Type : Common Manageability Programming Interface ( CMPI )
 *
 * Description: Provider for the Metric Indications.
 *
 */

#define CMPI_VERSION 90
#define _debug 1

#include "cmpidt.h"
#include "cmpift.h"
#include "cmpimacs.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include <mtrace.h>
#include <rrepos.h>
#include "OSBase_MetricUtil.h"

CMPIBroker * _broker;

/* ---------------------------------------------------------------------------*/
/* private declarations                                                       */

static char * _ClassName = "Linux_MetricIndication";
static char * _FILENAME = "OSBase_MetricIndicationProvider.c";

static char  _true=1;
static char  _false=0;

static pthread_mutex_t listenMutex = PTHREAD_MUTEX_INITIALIZER;

static int enabled = 0;
static int runningCorrelator = 100;

static int responsible(CMPISelectExp * filter,CMPIObjectPath * op, SubscriptionRequest * sr);
static int addListenFilter(CMPISelectExp * filter, SubscriptionRequest *sr);
static int removeListenFilter(CMPISelectExp * filter);
static void enableFilters();
static void disableFilters();

typedef struct _ListenFilter {
  int                    lf_enabled;
  CMPISelectExp *        lf_filter;
  SubscriptionRequest  * lf_subs;
  struct _ListenFilter * lf_next;
} ListenFilter;
static ListenFilter * listenFilters = NULL;

static int subscribeFilter(ListenFilter *lf);
static int unsubscribeFilter(ListenFilter *lf);
static void metricIndicationCB(int corrid, ValueRequest *vr);

/* ---------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------*/
/*                      Indication Provider Interface                             */
/* ---------------------------------------------------------------------------*/
CMPIStatus OSBase_MetricIndicationProviderIndicationCleanup( CMPIIndicationMI * mi, 
					       CMPIContext * ctx) 
{
  if( _debug )
    fprintf( stderr, "--- %s : %s CMPI IndicationCleanup()\n", _FILENAME, _ClassName );

  CMReturn(CMPI_RC_OK);
}

CMPIStatus OSBase_MetricIndicationProviderMustPoll
(CMPIIndicationMI * mi, CMPIContext * ctx, CMPIResult * res, CMPISelectExp * filter,
 const char * evtype, CMPIObjectPath * co)
{
  if( _debug )
    fprintf(stderr,"*** not polling for %s\n", _ClassName);
  CMReturnData(res, &_false, CMPI_boolean);
  CMReturnDone(res);
  CMReturn(CMPI_RC_OK);
}

CMPIStatus OSBase_MetricIndicationProviderAuthorizeFilter
(CMPIIndicationMI * mi, CMPIContext * ctx, CMPIResult * res, CMPISelectExp * filter,
 const char * evtype, CMPIObjectPath * co, const char * owner)
{
  if (responsible(filter,co,NULL)) {
    if( _debug )
      fprintf(stderr,"*** successfully authorized filter for %s\n", _ClassName);
    CMReturnData(res, &_true, CMPI_boolean);
  } else {
    if( _debug )
      fprintf(stderr,"*** refused to authorize filter for %s\n", _ClassName);
    CMReturnData(res, &_false, CMPI_boolean);
  }
  CMReturnDone(res);
  CMReturn(CMPI_RC_OK);
}

CMPIStatus OSBase_MetricIndicationProviderActivateFilter
(CMPIIndicationMI * mi, CMPIContext * ctx, CMPIResult * res, CMPISelectExp * filter,
 const char * evtype, CMPIObjectPath * co, CMPIBoolean first)
{
  SubscriptionRequest * sr = calloc(1,sizeof(SubscriptionRequest));
  if (responsible(filter,co,sr) && addListenFilter(filter,sr)== 0) {
    if( _debug )
      fprintf(stderr,"*** successfully activated filter for %s\n", _ClassName);
    CMReturn(CMPI_RC_OK);
  } else {
    free(sr);
    if( _debug )
      fprintf(stderr,"*** could not activate filter for %s\n", _ClassName);
    CMReturn(CMPI_RC_ERR_FAILED);
  }
}

CMPIStatus OSBase_MetricIndicationProviderDeActivateFilter
(CMPIIndicationMI * mi, CMPIContext * ctx, CMPIResult * res, CMPISelectExp * filter,
 const char * evtype, CMPIObjectPath * co, CMPIBoolean last)
{
  if (responsible(filter,co,NULL) && removeListenFilter(filter) == 0) {
    if( _debug )
      fprintf(stderr,"*** successfully deactivated filter for %s\n", _ClassName);
    CMReturn(CMPI_RC_OK);
  } else {
    if( _debug )
      fprintf(stderr,"*** could not deactivate filter for %s\n", _ClassName);
    CMReturn(CMPI_RC_ERR_FAILED);
  }
}

void OSBase_MetricIndicationProviderEnableIndications
(CMPIIndicationMI * mi)
{
  enableFilters();
  if( _debug )
    fprintf(stderr,"*** successfully enabled indications for %s\n", _ClassName);
}

void OSBase_MetricIndicationProviderDisableIndications
(CMPIIndicationMI * mi)
{
  disableFilters();
  if( _debug )
    fprintf(stderr,"*** successfully disabled indications for %s\n", _ClassName);
}


/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/

static void init_trace() {
#ifndef NOTRACE
  if (_debug)
    fprintf(stderr, "*** initialize provider tracing\n");
  m_trace_setlevel(4);
  m_trace_setfile("/tmp/repos_provider.trc");
  m_trace_enable(MTRACE_MASKALL);
#endif
}

CMIndicationMIStub( OSBase_MetricIndicationProvider, 
		    OSBase_MetricIndicationProvider, 
		    _broker, 
		    init_trace());


/* ---------------------------------------------------------------------------*/
/*                               Private Stuff                                */
/* ---------------------------------------------------------------------------*/

static int responsible(CMPISelectExp * filter, CMPIObjectPath *op, SubscriptionRequest * sr)
{
  if (op && filter) {
    CMPISelectCond * cond = CMGetDoc(filter,NULL);
    CMPIString     * condstring = filter->ft->getString(filter,NULL);
    if (!CMClassPathIsA(_broker,op,"Linux_MetricIndication",NULL)) {
      fprintf (stderr,"*** class path = %s\n",
	       CMGetCharPtr(CDToString(_broker,op,NULL)));
      return 0;
    }
    if (condstring && cond) {
      
      int i, j;
      CMPICount scount = CMGetSubCondCountAndType(cond,NULL,NULL);
      char * condarr = CMGetCharPtr(condstring);
      if (_debug)
	fprintf (stderr,"*** select expr = %s\n", condarr);
      if (_debug)
	fprintf (stderr, "*** got %d sub condition(s)\n",scount);
      for (i=0; i<scount; i++) {
	CMPISubCond * subcond = CMGetSubCondAt(cond,i,NULL);
	if (subcond) {
	  CMPICount pcount = CMGetPredicateCount(subcond,NULL);
	  if (_debug)
	    fprintf (stderr, "*** got %d predicate(s)\n",pcount);
	  for (j=0; j<pcount; j++) {
	    CMPIPredicate * pred = CMGetPredicateAt(subcond,j,NULL);
	    if (pred) {
	      CMPIType   type;
	      CMPIPredOp op;
	      CMPIString *lhs;
	      CMPIString *rhs;
	      CMGetPredicateData(pred,&type,&op,&lhs,&rhs);
	      if (strcasecmp("metricid",CMGetCharPtr(lhs))==0 && 
		  op == CMPI_PredOp_Equals) {
		/* we're in heaven :-) */
		if (_debug)
		  fprintf (stderr, "*** maps to subscription to %s\n", CMGetCharPtr(rhs));
		if (sr && rhs) {
		  char name[300];
		  parseMetricDefId(CMGetCharPtr(rhs),name,&sr->srMetricId);
		  sr->srCorrelatorId = runningCorrelator++;
		}
		return 1;
	      } 
	    }
	  }
	}
      }
    }
  }
  return 0;
}

static int addListenFilter(CMPISelectExp * filter, SubscriptionRequest *sr)
{
  ListenFilter *lf;
  pthread_mutex_lock(&listenMutex);
  lf = listenFilters;
  while (lf && lf->lf_next) {
    /* assuming no double filters */
    lf = lf->lf_next;
  }
  if (listenFilters == NULL) {
    lf = listenFilters = calloc(1,sizeof(ListenFilter));
  } else {
    lf->lf_next = calloc(1,sizeof(ListenFilter));
    lf = lf->lf_next;
  }
  lf->lf_filter = filter;
  lf->lf_subs = sr;
  if (enabled) {
    subscribeFilter(lf);
  }
  pthread_mutex_unlock(&listenMutex);
  return 0;
}

static int removeListenFilter(CMPISelectExp * filter)
{
  ListenFilter *lf;
  ListenFilter *prev;
  pthread_mutex_lock(&listenMutex);
  lf = listenFilters;  
  prev = lf;
  int state=1;
  while (lf) {
    if (lf->lf_filter==filter) {
      /* is the filter pointer the same as in add? */
      if (prev == listenFilters) {
	listenFilters = lf->lf_next;;
      } else {
	prev->lf_next = lf->lf_next;
      }
      if (lf->lf_enabled) {
	unsubscribeFilter(lf);
      }
      if (lf->lf_subs) {
	free (lf->lf_subs);
      }
      free(lf);
      state=0;
    }
    prev = lf;
    lf = lf->lf_next;
  }
  pthread_mutex_unlock(&listenMutex);
  return state;
}

void enableFilters()
{
  ListenFilter *lf;
  pthread_mutex_lock(&listenMutex);
  lf = listenFilters;
  while (lf) {
    if (!lf->lf_enabled) {
      if (_debug)
	fprintf (stderr, "*** enabling filter %p\n", lf->lf_filter);
      subscribeFilter(lf);
    }
    lf=lf->lf_next;
  }
  enabled = 1;
  pthread_mutex_unlock(&listenMutex);
}

void disableFilters()
{
  ListenFilter *lf;
  pthread_mutex_lock(&listenMutex);
  lf = listenFilters;
  while (lf) {
    if (lf->lf_enabled) {
      if (_debug)
	fprintf (stderr, "*** disabling filter %p\n", lf->lf_filter);
      unsubscribeFilter(lf);
    }
    lf=lf->lf_next;
  }
  enabled = 0;
  pthread_mutex_unlock(&listenMutex);
}

static int subscribeFilter(ListenFilter *lf)
{
  if (lf && lf->lf_subs && rrepos_subscribe(lf->lf_subs,metricIndicationCB)==0) {
    lf->lf_enabled = 1;
    if (_debug)
      fprintf (stderr, "*** subscribing filter %p\n", lf->lf_filter);
    return 0;
  }
  return 1;
}

static int unsubscribeFilter(ListenFilter *lf)
{
  if (lf && lf->lf_subs && rrepos_unsubscribe(lf->lf_subs,metricIndicationCB)==0) {
    lf->lf_enabled = 0;
    if (_debug)
      fprintf (stderr, "*** unsubscribing filter %p\n", lf->lf_filter);
    return 0;
  }
  return 1;
}

static void metricIndicationCB(int corrid, ValueRequest *vr)
{
  if (_debug)
    fprintf (stderr,"*** boo :-)\n");
}

/* ---------------------------------------------------------------------------*/
/*              end of OSBase_MetricIndicationProvider                             */
/* ---------------------------------------------------------------------------*/

