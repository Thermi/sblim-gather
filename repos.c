/*
 * $Id: repos.c,v 1.1 2004/07/09 15:20:52 mihajlov Exp $
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
 * Description: Gatherer Library
 * 
 * Runtime Control Functions.
 * For now we don't have synchronization features as we assume only one
 * control thread.
 */

#include "gather.h"
#include "mreg.h"
#include "mplugmgr.h"
#include "mlist.h"
#include "mretr.h"
#include "mrepos.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define min(a,b) ((a) < (b) ? (a) : (b))

/* Plugin Control */
typedef struct _PluginList {
  MetricPlugin       *plugin;
  struct _PluginList *next;
} PluginList;

static PluginList *pluginhead=NULL;

static void pl_link(MetricPlugin *);
static void pl_unlink(MetricPlugin *);
static MetricPlugin* pl_find(const char *);

/* Retriever Control */
static ML_Head   *metriclist=NULL;
static MR_Handle *retriever=NULL;

/* Sample Function */
static void gather_sample(int id);

/* Gatherer Interface Implementation */
int gather_init()
{
  /* Allocate all the data structures needed
     - plugin registry
     - retrievers 
  */
  if (metriclist) return -1;
  pluginhead = NULL;
  MPR_InitRegistry();
  metriclist = ML_Init();
  return 0;
}

int gather_terminate()
{
  /* stop everything */
  if (metriclist==NULL) return -1;
  gather_stop();
  while (pluginhead) {
    metricplugin_remove(pluginhead->plugin->mpName);
  };
  ML_Finish(metriclist);
  metriclist=NULL;
  MPR_FinishRegistry();
  return 0;
}

int gather_start()
{
  /* start the retrieval with one thread */
  if (metriclist && retriever==NULL) {
    ML_Reset(metriclist);
    retriever=MR_Init(metriclist,1);
    return 0;
  } else {
    return -1;
  }
}

int gather_stop()
{
  /* stop the retrieval */
  if (retriever) {
    MR_Finish(retriever);
    retriever=NULL;
    return 0;
  } else{
    return -1;
  }
}

void gather_status(GatherStatus *gs)
{
  if (gs) {
    PluginList *p=pluginhead;
    gs->gsNumPlugins=gs->gsNumMetrics=0;
    while(p && p->plugin) {
      gs->gsNumPlugins+=1;
      gs->gsNumMetrics+=p->plugin->mpNumMetricDefs;
      p=p->next;
    }
    gs->gsInitialized=metriclist!=NULL;
    gs->gsSampling=retriever!=NULL;
  }
}

int metricplugin_add(const char *pluginname)
{
  MetricPlugin *mp;
  int i;
  int status = -1;
  if (metriclist && pluginname && pl_find(pluginname)==NULL) {
    mp = malloc(sizeof(MetricPlugin));
    /* load plugin */
    mp->mpName = strdup(pluginname);
    mp->mpRegister=MPR_IdForString;
    if (MP_Load(mp)==0) {
      status = 0;
      pl_link(mp);
      /* register all metrics */
      for (i=0;i<mp->mpNumMetricDefs;i++)
	if (MPR_UpdateMetric(pluginname,mp->mpMetricDefs+i)==0 &&
	    (mp->mpMetricDefs[i].mdMetricType & MD_RETRIEVED)) {	  
	  MetricBlock *mb=MakeMB(mp->mpMetricDefs[i].mdId,
				 gather_sample,
				 mp->mpMetricDefs[i].mdSampleInterval);
	  if (mb==NULL || ML_Relocate(metriclist,mb))
	    status = -1;
	  else if (retriever)
	    /* notify retriever about new metrics */
	    MR_Wakeup(retriever);
	}
    } else {
      if(mp->mpName) free(mp->mpName);
      if(mp) free(mp);
    }
  }
  return status;
}

int metricplugin_remove(const char *pluginname)
{
  MetricPlugin *mp;
  int i;
  int status = -1;
  if (metriclist && pluginname) {
    mp = pl_find(pluginname);
    if (mp) {
      /* unregister all metrics for this plugin */
      for (i=0;i<mp->mpNumMetricDefs;i++) {
	if (mp->mpMetricDefs[i].mdMetricType & MD_RETRIEVED)
	  ML_Remove(metriclist,mp->mpMetricDefs[i].mdId);
	MPR_RemoveMetric(mp->mpMetricDefs[i].mdId);
      }
      pl_unlink(mp);
      MP_Unload(mp);
      free(mp->mpName);
      free(mp);
      status = 0;
    }
  }
  return status;
}

int metricplugin_list(const char *pluginname, PluginDefinition **pdef, 
		      COMMHEAP ch)
{
  MetricPlugin *mp;
  int i=-1;
  if (metriclist && pluginname && pdef) {
    mp = pl_find(pluginname);
    if (mp) {
      *pdef = ch_alloc(ch,sizeof(PluginDefinition)*mp->mpNumMetricDefs);
      /* store all metric infos for this plugin */
      for (i=0;i<mp->mpNumMetricDefs;i++) {
	(*pdef)[i].pdId=mp->mpMetricDefs[i].mdId;
	(*pdef)[i].pdDataType=mp->mpMetricDefs[i].mdDataType;
	(*pdef)[i].pdName=mp->mpMetricDefs[i].mdName;
	(*pdef)[i].pdResource=NULL; /* todo must specify resource listing fnc */
      }
    }
  }
  return i;
}

int metricvalue_get(ValueRequest *vs, COMMHEAP ch)
{
  MetricDefinition *md;
  MetricValue      **mv=NULL;
  int               i,j;
  int               id;
  char            **resources=NULL;
  int               resnum=0; 
  int              *numv=NULL;
  int               totalnum=0;
  int               actnum=0;
  int               useIntervals=0;
  int               intervalnum=0;
  
  if (vs) {
    md=MPR_GetMetric(vs->vsId);
    if (md && md->mcalc) {
      id = (md->mdMetricType&MD_CALCULATED) ? md->mdAliasId : vs->vsId;
      if  (vs->vsResource) {
	resources = &vs->vsResource;
	resnum = 1;
      } else if (md->mresl) {
	resnum = md->mresl(id,&resources);
      }
      if ( (md->mdMetricType&MD_INTERVAL) 
	   || (md->mdMetricType&MD_RATE) 
	   || (md->mdMetricType&MD_AVERAGE) ) {
	useIntervals=1;
	if(vs->vsFrom==vs->vsTo) { 
	  /* "point" interval */
	  if (md->mdMetricType&MD_INTERVAL)
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
	vs->vsDataType=md->mdDataType;
	for (j=0;j < resnum; j++) {
	  if (useIntervals && numv[j] > 0) {
	    vs->vsValues[j].viCaptureTime=mv[j][numv[j]-1].mvTimeStamp;
	    vs->vsValues[j].viDuration=
	      mv[j][0].mvTimeStamp -
	      vs->vsValues[j].viCaptureTime;
	    vs->vsValues[j].viValueLen=100; /* TODO : calc meaningful length */
	    vs->vsValues[j].viValue=ch_alloc(ch,vs->vsValues[j].viValueLen);
	    if (md->mcalc(mv[j],
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
	      if (md->mcalc(&mv[j][i],
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
	if (vs->vsResource == NULL && resources && md->mresldeal) {
	  md->mresldeal(resources);
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

static void pl_link(MetricPlugin *mp)
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
  p->plugin = mp;
  p->next = NULL;
}

static void pl_unlink(MetricPlugin *mp)
{
  PluginList *p, *q;
  p = pluginhead;
  if (p && p->plugin==mp) {
    pluginhead=p->next;
    free(p);
  } else
    while (p->next) {
      if (p->next->plugin==mp) {
	q=p->next;
	p->next=q->next;
	free(q);
	break;
      }
      p=p->next;
    }
}

static MetricPlugin* pl_find(const char *name)
{
  PluginList *p = pluginhead;
  while(p) {
    if (strcmp(p->plugin->mpName,name)==0)
      break;
    p=p->next;
  }
  return p?p->plugin:NULL;
}

static void gather_sample(int id)
{
  MetricDefinition *md;
  
  md=MPR_GetMetric(id);
  if (md && md->mproc) {
    md->mproc(id,MetricRepository->mrep_add);
  }
}
