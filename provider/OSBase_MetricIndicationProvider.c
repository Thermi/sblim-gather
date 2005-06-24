
/*
 * $Id: OSBase_MetricIndicationProvider.c,v 1.4 2005/06/24 12:04:56 mihajlov Exp $
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
#ifdef DEBUG
#define _debug 1
#else
#define _debug 0
#endif


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
static pthread_key_t   listen_key;
static pthread_once_t  listen_once = PTHREAD_ONCE_INIT;
static CMPIContext   * listenContext = NULL;

static int enabled = 0;
static int runningCorrelator = 100;

static int responsible(CMPISelectExp * filter,CMPIObjectPath * op, SubscriptionRequest * sr);
static int addListenFilter(CMPISelectExp * filter, CMPIObjectPath * op, SubscriptionRequest *sr);
static int removeListenFilter(CMPISelectExp * filter);
static void enableFilters();
static void disableFilters();

typedef struct _ListenFilter {
  int                    lf_enabled;
  CMPISelectExp *        lf_filter;
  SubscriptionRequest  * lf_subs;
  char                 * lf_namespace;
  struct _ListenFilter * lf_next;
} ListenFilter;
static ListenFilter * listenFilters = NULL;

static int subscribeFilter(ListenFilter *lf);
static int unsubscribeFilter(ListenFilter *lf);
static void metricIndicationCB(int corrid, ValueRequest *vr);

static void listen_term(void *voidctx);
static void listen_init();
static CMPIContext * attachListenContext();


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
  if (responsible(filter,co,sr)) {
    /* Prepare attachment of secondary threads */
    if (listenContext == NULL) {
      if( _debug )
	fprintf(stderr,"*** preparing seondary thread attach\n");
      listenContext = CBPrepareAttachThread(_broker,ctx);
    }
    if (addListenFilter(filter,co,sr)== 0) {
      if( _debug )
	fprintf(stderr,"*** successfully activated filter for %s\n", _ClassName);
      CMReturn(CMPI_RC_OK);
    } else {
      /* was not freed in addListenFilter */
      free(sr);
    }
  }
  if( _debug )
    fprintf(stderr,"*** could not activate filter for %s\n", _ClassName);
  CMReturn(CMPI_RC_ERR_FAILED);
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
	  for (j=pcount-1; j>=0; j--) {
	    CMPIPredicate * pred = CMGetPredicateAt(subcond,j,NULL);
	    if (pred) {
	      CMPIType   type;
	      CMPIPredOp op;
	      CMPIString *lhs=NULL;
	      CMPIString *rhs=NULL;
	      if (_debug)
		fprintf (stderr, "*** checking predicate\n");
	      CMGetPredicateData(pred,&type,&op,&lhs,&rhs);
	      if (_debug)
		fprintf (stderr, "*** lhs=%s, rhs=%s\n",CMGetCharPtr(lhs),
			 CMGetCharPtr(rhs));
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

static int addListenFilter(CMPISelectExp * filter, CMPIObjectPath * op, SubscriptionRequest *sr)
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
  lf->lf_namespace = strdup(CMGetCharPtr(CMGetNameSpace(op,NULL)));
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
      if (lf->lf_namespace) {
	free (lf->lf_namespace);
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
  CMPIObjectPath * co;
  CMPIInstance   * ci;
  CMPIContext    * ctx;
  ListenFilter   * lf;
  CMPIDateTime   * datetime;
  CMPIString     * metricvalue;
  char             mvalId[1000];
  char             mdefId[1000];
  if (_debug)
    fprintf (stderr,"*** metric indication callback triggered\n");

  ctx = attachListenContext();
  if (ctx==NULL) {
    if (_debug)
      fprintf (stderr,"*** failed to get thread context \n");
    return;
  }
  
  lf = listenFilters;
  while (lf) {
    if (lf->lf_enabled && lf->lf_subs && lf->lf_subs->srCorrelatorId==corrid) {
      break;
    }
    lf = lf->lf_next;
  }

  if (lf) {
    co = CMNewObjectPath(_broker,lf->lf_namespace,_ClassName,NULL);
    if (co &&  makeMetricValueIdFromCache(_broker, ctx, lf->lf_namespace,
					  mvalId,vr->vsId,
					  vr->vsValues->viResource,
					  vr->vsValues->viSystemId,
					  vr->vsValues->viCaptureTime)) {
      ci = CMNewInstance(_broker,co,NULL);
      if (ci) {
	makeMetricDefIdFromCache(_broker,ctx,lf->lf_namespace,mdefId,vr->vsId);
	CMSetProperty(ci,"IndicationIdentifier",mvalId,CMPI_chars);
	CMSetProperty(ci,"MetricId",mdefId,CMPI_chars);
	metricvalue = val2string(_broker,vr->vsValues,vr->vsDataType);
	if (metricvalue)
	  CMSetProperty(ci,"MetricValue",&metricvalue,CMPI_string);
	datetime = 
	  CMNewDateTimeFromBinary(_broker,
				  (long long)vr->vsValues->viCaptureTime*1000000,
				  0, NULL);
	if (datetime)
	  CMSetProperty(ci,"IndicationTime",&datetime,CMPI_dateTime);
	if (_debug)
	  fprintf (stderr,"*** delivering metric indication\n");
	CBDeliverIndication(_broker,ctx,lf->lf_namespace,ci);
      }      
    } else { 
      if (_debug)
	fprintf (stderr,"*** failed to construct indication\n");
    }
  } else {
    if (_debug)
      fprintf (stderr,"*** received uncorrelated event\n");
  }
}

static void listen_term(void *voidctx)
{
  /* detach thread */
  CMPIContext *ctx;
  if (_debug)
    fprintf (stderr,"*** detaching thread from CMPI\n");
  ctx = (CMPIContext*)voidctx;
  CBDetachThread(_broker,ctx);
}

static void listen_init()
{
  pthread_key_create(&listen_key,listen_term);
}

static CMPIContext* attachListenContext()
{
  /* attach CMPI to current thread if required */
  CMPIContext *ctx;
  pthread_once(&listen_once,listen_init);
  ctx = (CMPIContext*)pthread_getspecific(listen_key);
  if (ctx == NULL && listenContext) {
    if (_debug)
      fprintf (stderr,"*** attaching thread to CMPI\n");
    CBAttachThread(_broker,listenContext);
    ctx = listenContext;
    pthread_setspecific(listen_key,ctx); 
  }
  return ctx;
}

/* ---------------------------------------------------------------------------*/
/*              end of OSBase_MetricIndicationProvider                             */
/* ---------------------------------------------------------------------------*/

