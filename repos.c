/*
 * $Id: repos.c,v 1.2 2004/07/16 15:30:04 mihajlov Exp $
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
  if (pluginname && pl_find(pluginname)==NULL) {
    rp = malloc(sizeof(RepositoryPlugin));
    /* load plugin */
    rp->rpName = strdup(pluginname);
    rp->rpRegister=RPR_IdForString;
    if (RP_Load(rp)==0) {
      status = 0;
      pl_link(rp);
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
	(*rdef)[i].rdName=rp->rpMetricCalcDefs[i].mcName;
	(*rdef)[i].rdResource=NULL; /* todo must specify resource listing fnc */
      }
    }
  }
  return i;
}

int reposvalue_put(ValueRequest *vs, COMMHEAP ch)
{
  return -1;
}

int reposvalue_get(ValueRequest *vs, COMMHEAP ch)
{
  MetricCalculationDefinition *mc;
  MetricValue                **mv=NULL;
  int                          i,j;
  int                          id;
  char                       **resources=NULL;
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
      if  (vs->vsResource) {
	resources = &vs->vsResource;
	resnum = 1;
      } else {
	resnum = MetricRepository->mres_retrieve(id,&resources);
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
	  if (MetricRepository-> mrep_retrieve(id,
					       resources[j],
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
	    vs->vsValues[j].viCaptureTime=mv[j][numv[j]-1].mvTimeStamp;
	    vs->vsValues[j].viDuration=
	      mv[j][0].mvTimeStamp -
	      vs->vsValues[j].viCaptureTime;
	    vs->vsValues[j].viValueLen=100; /* TODO : calc meaningful length */
	    vs->vsValues[j].viValue=ch_alloc(ch,vs->vsValues[j].viValueLen);
	    if (mc->mcCalc(mv[j],
			  numv[j],
			  vs->vsValues[j].viValue,
			  vs->vsValues[j].viValueLen) == -1) {
	      /* failed to obtain value */
	      resnum -= 1;
	      vs->vsNumValues -= 1;
	      continue;
	    }	      
	    vs->vsValues[j].viResource=ch_alloc(ch,strlen(resources[j])+1);
	    strcpy(vs->vsValues[j].viResource,resources[j]);
	  } else {	
	    for (i=0; i < numv[j]; i++) {
	      vs->vsValues[actnum+i].viCaptureTime=mv[j][i].mvTimeStamp;
	      vs->vsValues[actnum+i].viDuration=0;
	      vs->vsValues[actnum+i].viValueLen=100;
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
	      vs->vsValues[actnum+i].viResource=
		ch_alloc(ch,strlen(resources[j])+1);
	      strcpy(vs->vsValues[actnum+i].viResource,resources[j]);
	    }
	    actnum = actnum + numv[j];
	  }
	}
	if (vs->vsResource == NULL && resources) {
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
