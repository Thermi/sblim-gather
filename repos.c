/*
 * $Id: repos.c,v 1.13 2004/11/26 15:25:34 mihajlov Exp $
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
 * Description: Repository Library
 * 
 * Runtime Control Functions.
 * For now we don't have synchronization features as we assume only one
 * control thread.
 */

#include "repos.h"
#include "rreg.h"
#include "rplugmgr.h"
#include "mrepos.h"
#include "mtrace.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>

/* Plugin Control -- copied over from gather.c */
typedef struct _PluginList {
  RepositoryPlugin       *plugin;
  struct _PluginList *next;
} PluginList;

static PluginList *pluginhead=NULL;
static size_t      pluginnum=0;
static int         initialized=0;

static void pl_link(RepositoryPlugin *);
static void pl_unlink(RepositoryPlugin *);
static RepositoryPlugin* pl_find(const char *);

/* Subscription Support */
static void RepositorySubscriptionCallback(MetricValue *mv);
typedef struct _RepositorySubscription {
  SubscriptionRequest            *rsr_req;
  SubscriptionCallback           *rsr_cb;
  struct _RepositorySubscription *rsr_next;
} RepositorySubscription;

static RepositorySubscription *subscriptions=NULL;
static int sub_add(SubscriptionRequest *, SubscriptionCallback*);
static int sub_remove(SubscriptionRequest *, SubscriptionCallback*);
static int  matchCommonCriteria(SubscriptionRequest *, MetricValue*);
static int  matchValue(SubscriptionRequest *, ValueRequest *);

int repos_init()
{
  /* Allocate all the data structures needed
     - plugin registry
     - retrievers 
  */
  if (initialized==0) {
    initialized=1;
    pluginhead = NULL;
    RPR_InitRegistry();
    return 0;
  }
  return -1;
}

int repos_terminate()
{
  if (initialized) {
    /* stop everything */
    initialized=0;
    while (pluginhead) {
      reposplugin_remove(pluginhead->plugin->rpName);
    };
    RPR_FinishRegistry();
    return 0;
  }
  return -1;
}

int repos_sessiontoken(RepositoryToken *rt)
{
  if (rt) {
    rt->rt_size=htonl(sizeof(RepositoryToken));
    rt->rt1 = 1234567;
    rt->rt1 = 7654321;
    return 0;
  }
  return -1;
}

void repos_status(RepositoryStatus *rs)
{
  if (rs) {
    PluginList *p=pluginhead;
    rs->rsNumPlugins=rs->rsNumMetrics=0;
    while(p && p->plugin) {
      rs->rsNumPlugins+=1;
      rs->rsNumMetrics+=p->plugin->rpNumMetricCalcDefs;
      p=p->next;
    }
    rs->rsInitialized=initialized;
  }
}

int reposplugin_add(const char *pluginname)
{
  RepositoryPlugin *rp;
  int status = -1;
  int i;
  if (pluginname && pl_find(pluginname)==NULL) {
    rp = malloc(sizeof(RepositoryPlugin));
    /* load plugin */
    rp->rpName = strdup(pluginname);
    rp->rpRegister=RPR_IdForString;
    if (RP_Load(rp)==0) {
      status = 0;
      pl_link(rp);
      /* register all metrics */
      for (i=0;i<rp->rpNumMetricCalcDefs;i++) {
	if (rp->rpMetricCalcDefs[i].mcVersion > 
	    (MD_VERSION_MAJOR+MD_VERSION_MINOR_MAX)) {
	  status=-1;
	  break;
	} 
	if (RPR_UpdateMetric(pluginname,rp->rpMetricCalcDefs+i)!=0) {
	  status = -1;
	  break;
	}
      }
      if (status<0) {
	/* failed during registration - unload */
	reposplugin_remove(pluginname);
	return status;
      }
    } else {
      if(rp->rpName) free(rp->rpName);
      if(rp) free(rp);
    }
  }
  return status;
}

int reposplugin_remove(const char *pluginname)
{
  RepositoryPlugin *rp;
  int i;
  int status = -1;
  if (pluginname) {
    rp = pl_find(pluginname);
    if (rp) {
      /* unregister all metrics for this plugin */
      for (i=0;i<rp->rpNumMetricCalcDefs;i++) {
	RPR_RemoveMetric(rp->rpMetricCalcDefs[i].mcId);
      }
      pl_unlink(rp);
      RP_Unload(rp);
      free(rp->rpName);
      free(rp);
      status = 0;
    }
  }
  return status;
}

int reposplugin_list(const char *pluginname, 
		     RepositoryPluginDefinition **rdef, 
		     COMMHEAP ch)
{
  RepositoryPlugin *rp;
  int i=-1;
  if (pluginname && rdef) {
    rp = pl_find(pluginname);
    if (rp) {
      *rdef = 
	ch_alloc(ch,
		 sizeof(RepositoryPluginDefinition)*rp->rpNumMetricCalcDefs);
      /* store all metric infos for this plugin */
      for (i=0;i<rp->rpNumMetricCalcDefs;i++) {
	(*rdef)[i].rdId=rp->rpMetricCalcDefs[i].mcId;
	(*rdef)[i].rdDataType=rp->rpMetricCalcDefs[i].mcDataType;
	(*rdef)[i].rdMetricType=rp->rpMetricCalcDefs[i].mcMetricType;
	(*rdef)[i].rdChangeType=rp->rpMetricCalcDefs[i].mcChangeType;
	(*rdef)[i].rdIsContinuous=rp->rpMetricCalcDefs[i].mcIsContinuous;
	(*rdef)[i].rdName=rp->rpMetricCalcDefs[i].mcName;
	(*rdef)[i].rdResource=NULL; /* todo must specify resource listing fnc */
      }
    }
  }
  return i;
}

int reposresource_list(const char * metricid,
		       MetricResourceId **rid,
		       COMMHEAP ch)
{
  MetricResourceId *resources=NULL;
  int resnum=0;
  int i;

  if (atoi(metricid)==-1 || !rid) { return -1; }

  resnum = MetricRepository->mres_retrieve(atoi(metricid),
					   &resources,
					   NULL,
					   NULL);
  if(resnum) {
    *rid = ch_alloc(ch,
		    sizeof(MetricResourceId)*resnum);
    /* store all resource infos for this metric id */
    for (i=0;i<resnum;i++) {
      (*rid)[i].mrid_resource =	ch_alloc(ch,strlen(resources[i].mrid_resource)+1);	    
      strcpy((*rid)[i].mrid_resource,
	     resources[i].mrid_resource);
      (*rid)[i].mrid_system = ch_alloc(ch,strlen(resources[i].mrid_system)+1);	    
      strcpy((*rid)[i].mrid_system,
	     resources[i].mrid_system);
    }
    if (resources) {
      MetricRepository->mres_release(resources);
    }
    return resnum;
  }
  return -1;
}

int reposvalue_put(const char *reposplugin, const char *metric, 
		   MetricValue *mv)
{
  int id = RPR_IdForString(reposplugin,metric);
  if (id > 0) {
    mv->mvId=id; /* we are overwriting the caller's argument in good faith */
    return MetricRepository->mrep_add(mv);
  }
  return -1;
}

int reposvalue_get(ValueRequest *vs, COMMHEAP ch)
{
  MetricCalculationDefinition *mc;
  MetricValue                **mv=NULL;
  int                          i,j;
  int                          id;
  MetricResourceId             singleResource;
  MetricResourceId             *resources=NULL;
  int                          resnum=0; 
  int                         *numv=NULL;
  int                          totalnum=0;
  int                          actnum=0;
  int                          useIntervals=0;
  int                          intervalnum=0;
  
  if (vs) {
    mc=RPR_GetMetric(vs->vsId);
    if (mc && mc->mcCalc) {
      id = (mc->mcMetricType&MD_CALCULATED) ? mc->mcAliasId : vs->vsId;
      if  (vs->vsResource && vs->vsSystemId) {
	resources = &singleResource;
	resources->mrid_resource = vs->vsResource;
	resources->mrid_system = vs->vsSystemId;
	resnum = 1;
      } else {
	resnum = MetricRepository->mres_retrieve(id,&resources,
						 vs->vsResource,
						 vs->vsSystemId);
      }
      if ( (mc->mcMetricType&MD_INTERVAL) 
	   || (mc->mcMetricType&MD_RATE) 
	   || (mc->mcMetricType&MD_AVERAGE) ) {
	useIntervals=1;
	if(vs->vsFrom==vs->vsTo) { 
	  /* "point" interval */
	  if (mc->mcMetricType&MD_INTERVAL)
	    intervalnum = 1;
	  else
	    intervalnum = 2;
	}
      }
      if (resnum) {
	mv = calloc(resnum, sizeof(MetricValue*));
	numv = calloc(resnum,sizeof(int));
	for (j=0; j < resnum; j++) {
	  if (MetricRepository->mrep_retrieve(id,
					      resources + j,
					      &mv[j],
					      &numv[j],
					      vs->vsFrom,
					      vs->vsTo,
					      intervalnum) != -1 ) {
	    totalnum += numv[j];
	  }
	}
	if (useIntervals) {
	  /* here the interval-type metrics are computed - by resource */
	  vs->vsNumValues=resnum; /* one per resource */
	  for (j=0; j < resnum; j++) {
	    if (intervalnum && (numv[j]<intervalnum)) {
	      numv[j] = 0;  /* this value cannot be computed */
	    }
	    if (numv[j]==0) {
	      vs->vsNumValues-=1; 
	    }
	  }
	} else {
	  vs->vsNumValues=totalnum; /* all values used for point metrics */
	}  
	vs->vsValues=ch_alloc(ch,vs->vsNumValues*sizeof(ValueItem));
	vs->vsDataType=mc->mcDataType;
	for (j=0;j < resnum; j++) {
	  if (useIntervals && numv[j] > 0) {
	    vs->vsValues[actnum].viCaptureTime=mv[j][numv[j]-1].mvTimeStamp;
	    vs->vsValues[actnum].viDuration=
	      mv[j][0].mvTimeStamp -
	      vs->vsValues[actnum].viCaptureTime;
	    vs->vsValues[actnum].viValueLen=100; /* TODO : calc meaningful length */
	    vs->vsValues[actnum].viSystemId=
	      ch_alloc(ch,strlen(mv[j][numv[j]-1].mvSystemId)+1);	    
	    strcpy(vs->vsValues[actnum].viSystemId,
		   mv[j][numv[j]-1].mvSystemId);
	    vs->vsValues[actnum].viValue=
	      ch_alloc(ch,vs->vsValues[actnum].viValueLen);
	    if (mc->mcCalc(mv[j],
			  numv[j],
			  vs->vsValues[actnum].viValue,
			  vs->vsValues[actnum].viValueLen) == -1) {
	      /* failed to obtain value */
	      resnum -= 1;
	      vs->vsNumValues -= 1;
	      continue;
	    }
	    /* Q: Shouldn't we use the resource from the value ?*/
	    vs->vsValues[actnum].viResource=
	      ch_alloc(ch,strlen(resources[j].mrid_resource)+1);
	    strcpy(vs->vsValues[actnum].viResource,
		   resources[j].mrid_resource);
	    actnum += 1;
	  } else {	
	    for (i=0; i < numv[j]; i++) {
	      vs->vsValues[actnum+i].viCaptureTime=mv[j][i].mvTimeStamp;
	      vs->vsValues[actnum+i].viDuration=0;
	      vs->vsValues[actnum+i].viValueLen=100;
	      vs->vsValues[actnum+i].viSystemId=
		ch_alloc(ch,strlen(mv[j][i].mvSystemId)+1);	    
	      strcpy(vs->vsValues[actnum+i].viSystemId,
		     mv[j][i].mvSystemId);
	      vs->vsValues[actnum+i].viValue=
		ch_alloc(ch,vs->vsValues[actnum+i].viValueLen);
	      if (mc->mcCalc(&mv[j][i],
			    1,
			    vs->vsValues[actnum+i].viValue,
			    vs->vsValues[actnum+i].viValueLen) == -1) {
		/* failed to obtain value */
		numv[j] -= 1;
		vs->vsNumValues -= 1;
		continue;
	      }	      
	      /* Q: Shouldn't we use the resource from the value ?*/
	      vs->vsValues[actnum+i].viResource=
		ch_alloc(ch,strlen(resources[j].mrid_resource)+1);
	      strcpy(vs->vsValues[actnum+i].viResource,
		     resources[j].mrid_resource);
	    }
	    actnum = actnum + numv[j];
	  }
	}
	if (vs->vsResource == NULL && vs->vsSystemId == NULL &&  resources) {
	  MetricRepository->mres_release(resources);
	}
	for (j=0; j < resnum; j++) {
	  MetricRepository->mrep_release(mv[j]);
	}
	if (numv) free(numv);
	if (mv) free(mv);
	if (vs->vsNumValues > 0) return 0;
      }
    }
  }
  return -1;
}

int repos_unsubscribe(SubscriptionRequest *sr, SubscriptionCallback *scb)
{
  M_TRACE(MTRACE_FLOW,MTRACE_REPOS,
	  ("repos_unsubscribe %p %p", sr, scb));
  if (sr && scb && sub_remove(sr,scb) == 0) {
    return MetricRepository->
      mrep_regcallback(RepositorySubscriptionCallback,
			 sr->srBaseMetricId,
			 MCB_STATE_UNREGISTER);
  };
  M_TRACE(MTRACE_ERROR,MTRACE_REPOS,
	  ("repos_unsubscribe invalid parameter %p %p", sr, scb));
  return -1;
}

int repos_subscribe(SubscriptionRequest *sr, SubscriptionCallback *scb)
{
  M_TRACE(MTRACE_FLOW,MTRACE_REPOS,
	  ("repos_subscribe %p %p", sr, scb));
  if (sr && scb && sub_add(sr,scb) == 0) {
    return MetricRepository->
      mrep_regcallback(RepositorySubscriptionCallback,
		       sr->srBaseMetricId,
		       MCB_STATE_REGISTER);
  };
  M_TRACE(MTRACE_ERROR,MTRACE_REPOS,
	  ("repos_subscribe invalid parameter %p %p", sr, scb));
  return -1;
}

static void RepositorySubscriptionCallback(MetricValue *mv)
{
  M_TRACE(MTRACE_FLOW,MTRACE_REPOS,
	  ("RepositorySubscriptionCallback %p (%d)", mv, mv?mv->mvId:-1));
  if (mv) {
    MetricCalculationDefinition *mc=NULL;
    RepositorySubscription *subs = subscriptions;
    while (subs && subs->rsr_req) {
      ValueRequest vr;
      ValueItem    vi;
      char         valbuf[100];
      if (subs->rsr_req->srBaseMetricId > mv->mvId) {
	break;
      }
      if (matchCommonCriteria(subs->rsr_req,mv)) {
	/* NOTE: at the moment we handle only point metrics (correctly) */
	/* calculate if common criteria matches */
	mc=RPR_GetMetric(subs->rsr_req->srMetricId);
	if (mc && mc->mcCalc) {
	  vr.vsId=subs->rsr_req->srMetricId;
	  vr.vsSystemId=vr.vsResource=NULL;
	  vr.vsNumValues=1;
	  vr.vsValues=&vi;
	  vi.viCaptureTime=mv->mvTimeStamp;
	  vi.viDuration=0;
	  vi.viResource=mv->mvResource;
	  vi.viSystemId=mv->mvSystemId;
	  vi.viValueLen=sizeof(valbuf);
	  vi.viValue=valbuf;
	  if (mc->mcCalc(mv,1,vi.viValue,vi.viValueLen) == -1) {
	    M_TRACE(MTRACE_ERROR,MTRACE_REPOS,
		    ("RepositorySubscriptionCallback failed to calculate metric"
		     " value for %d", subs->rsr_req->srMetricId));
	  } else  if (matchValue(subs->rsr_req,&vr)) {
	    subs->rsr_cb(subs->rsr_req->srCorrelatorId,&vr);
	  }
	} else {
	  M_TRACE(MTRACE_ERROR,MTRACE_REPOS,
		  ("RepositorySubscriptionCallback failed to retrieve metric"
		   " calculator for %d", subs->rsr_req->srMetricId));
	}
      }
      subs = subs->rsr_next;
    }
  }
}

static int sub_add(SubscriptionRequest *sr, SubscriptionCallback *scb)
{
  /* TODO: add locks */
  if (sr && scb) {
    RepositorySubscription *subs = subscriptions;
    RepositorySubscription *prev = subscriptions;
    MetricCalculationDefinition *mc=RPR_GetMetric(sr->srMetricId);
    if (mc == NULL) {
      M_TRACE(MTRACE_ERROR,MTRACE_REPOS,
	      ("sub_add failed to retrieve metric"
	       " definition for %d", sr->srMetricId));
      return -1;
    }
    sr->srBaseMetricId = 
      (mc->mcMetricType&MD_CALCULATED) ? mc->mcAliasId : sr->srMetricId;
    while (subs && subs->rsr_req) {
      if (subs->rsr_req->srMetricId == sr->srMetricId &&
	  subs->rsr_req->srBaseMetricId == sr->srBaseMetricId &&
	  subs->rsr_cb == scb) {
	/* already in list */
	return 0;
      } else if (subs->rsr_next && 
		 subs->rsr_next->rsr_req->srBaseMetricId > 
		 sr->srBaseMetricId) {
	/* the list is sorted by BaseMetricIds for better search performance */
	break;
      }
      prev = subs;
      subs = subs->rsr_next;
    }
    subs = malloc(sizeof(RepositorySubscription));
    subs->rsr_req = malloc(sizeof(SubscriptionRequest));
    *subs->rsr_req = *sr;
    subs->rsr_req->srValue = sr->srValue ? strdup(sr->srValue) : NULL;
    subs->rsr_req->srSystemId = sr->srSystemId ? strdup(sr->srSystemId) : NULL;
    subs->rsr_req->srResource = sr->srResource ? strdup(sr->srResource) : NULL;
    subs->rsr_cb = scb;
    if (subscriptions==prev && 
	(subscriptions == NULL || subscriptions->rsr_req->srBaseMetricId > 
	 subs->rsr_req->srBaseMetricId) ) {
      subs->rsr_next = subscriptions;
      subscriptions=subs;
    } else {
      subs->rsr_next = prev->rsr_next;
      prev->rsr_next=subs;
    }
    return 0;
  }
  M_TRACE(MTRACE_ERROR,MTRACE_REPOS,
	  ("sub_add invalid parameters %p %p", sr, scb));
  return -1;
}

static int sub_remove(SubscriptionRequest *sr, SubscriptionCallback *scb)
{
  /* TODO: add locks */
  if (sr) {
    RepositorySubscription *subs = subscriptions;
    RepositorySubscription *prev = subscriptions;
    MetricCalculationDefinition *mc=RPR_GetMetric(sr->srMetricId);
    if (mc == NULL) {
      M_TRACE(MTRACE_ERROR,MTRACE_REPOS,
	      ("sub_remove failed to retrieve metric"
	       " definition for %d", sr->srMetricId));
      return -1;
    }
    sr->srBaseMetricId = 
      (mc->mcMetricType&MD_CALCULATED) ? mc->mcAliasId : sr->srMetricId;
    while (subs && subs->rsr_req) {
      if (subs->rsr_req->srMetricId == sr->srMetricId &&
	  subs->rsr_req->srBaseMetricId == sr->srBaseMetricId &&
	  subs->rsr_cb == scb) {
	/* unlink */
	if (prev == subs) {
	  subscriptions = subs->rsr_next;
	} else {
	  prev->rsr_next = subs->rsr_next;
	}
	if (subs->rsr_req->srValue) {
	  free(subs->rsr_req->srValue);
	}
	if (subs->rsr_req->srSystemId) {
	  free(subs->rsr_req->srSystemId);
	}
	if (subs->rsr_req->srResource) {
	  free(subs->rsr_req->srResource);
	}
	free(subs->rsr_req);
	return 0;
      } else if (subs->rsr_next && 
		 subs->rsr_next->rsr_req->srBaseMetricId > 
		 sr->srBaseMetricId) {
	/* not in list */
	break;
      }
      prev = subs;
      subs = subs->rsr_next;
    }
    M_TRACE(MTRACE_ERROR,MTRACE_REPOS,
	    ("sub_remove subscription not found (%d,%d)", 
	     sr->srMetricId, sr->srCorrelatorId));
  }
  M_TRACE(MTRACE_ERROR,MTRACE_REPOS,
	  ("sub_remove invalid parameters %p %p", sr, scb));
  return -1;
}

static int  matchCommonCriteria(SubscriptionRequest * sr, MetricValue* mv)
{
  int match = 0;
  if (sr && mv) {
    /* handle primary and computed metrics */
    if (sr->srMetricId == mv->mvId || sr->srBaseMetricId == mv->mvId) {
      switch( sr->srSystemOp ) {
      case SUBSCR_OP_ANY:
	match = 1;
	break;
      case SUBSCR_OP_EQ:
	match = !strcmp(sr->srSystemId,mv->mvSystemId);
	break;
      case SUBSCR_OP_NE:
	match = strcmp(sr->srSystemId,mv->mvSystemId);
	break;
      default:
	match = 0;
      }
      if (match) {
	switch( sr->srResourceOp ) {
	case SUBSCR_OP_ANY:
	  match = 1;
	  break;
	case SUBSCR_OP_EQ:
	  match = !strcmp(sr->srResource,mv->mvResource);
	  break;
	case SUBSCR_OP_NE:
	  match = strcmp(sr->srResource,mv->mvResource);
	  break;
	default:
	  match = 0;
	}
      }
    }
  }
  return match;
}


static int  matchValue(SubscriptionRequest * sr, ValueRequest * vr)
{
  int compstate;
  if (sr && vr && vr->vsValues && vr->vsValues->viValue) {
    if ( sr->srValueOp == SUBSCR_OP_ANY ) {
      return 1;
    }
    if (vr->vsDataType & (MD_BOOL|MD_UINT|MD_SINT)) {
      long long ls = atoll(sr->srValue);
      long long lv = *(long long*)vr->vsValues->viValue;
      compstate = ls - lv; 
    } else if (vr->vsDataType & (MD_FLOAT)) {
      double ds = atof(sr->srValue);
      double dv = *(double*)vr->vsValues->viValue;
      compstate = ds - dv; 
    } else {
      compstate = strcmp(sr->srValue,vr->vsValues->viValue);
    }
    switch( sr->srValueOp ) {
    case SUBSCR_OP_EQ:
      return compstate == 0;
    case SUBSCR_OP_NE:
      return compstate != 0;
    case SUBSCR_OP_GT:
      return compstate > 0;
    case SUBSCR_OP_GE:
      return compstate >= 0;
    case SUBSCR_OP_LT:
      return compstate < 0;
    case SUBSCR_OP_LE:
      return compstate <= 0;
    }
  }
  
  return 0;
}

static void pl_link(RepositoryPlugin *rp)
{
  PluginList *p = pluginhead;
  if (p == NULL) {
    pluginhead = malloc(sizeof(PluginList));
    p = pluginhead;
  } else {
    while (p->next)
      p=p->next;
    p->next=malloc(sizeof(PluginList));
    p=p->next;
  }
  p->plugin = rp;
  p->next = NULL;
  pluginnum+=1;
}

static void pl_unlink(RepositoryPlugin *rp)
{
  PluginList *p, *q;
  p = pluginhead;
  if (p && p->plugin==rp) {
    pluginhead=p->next;
    free(p);
    pluginnum-=1;
  } else
    while (p->next) {
      if (p->next->plugin==rp) {
	q=p->next;
	p->next=q->next;
	free(q);
	pluginnum-=1;
	break;
      }
      p=p->next;
    }
}

static RepositoryPlugin* pl_find(const char *name)
{
  PluginList *p = pluginhead;
  while(p) {
    if (strcmp(p->plugin->rpName,name)==0)
      break;
    p=p->next;
  }
  return p?p->plugin:NULL;
}
