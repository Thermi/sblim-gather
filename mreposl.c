/*
 * $Id: mreposl.c,v 1.8 2004/12/01 16:13:33 mihajlov Exp $
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
 * Author:       Viktor Mihajlovski <mihajlov@de.ibm.cim>
 * Contributors: 
 *
 * Description: Local Metric Value Repository
 * This is an implementation of a history-less (well it caches 15 minutes)
 * and local repository. Others will follow.
 */

#include "mrepos.h"
#include "mrwlock.h"
#include "mtrace.h" 
#include "mreg.h" 

#include <stdlib.h>
#include <limits.h>
#include <string.h>


/* TODO: must revise repository IF to not require resource listing */

static MetricResourceId * reslist = NULL;
static size_t  resnum = 0;
static int findres(char *, char *);
static void addres(char *, char *);

int LocalMetricResources(int id, MetricResourceId ** resources, 
			 const char * resource, const char * system);
int LocalMetricFreeResources(MetricResourceId * resources);
  

/* Local repository functions */

int LocalMetricAdd (MetricValue *mv);
int LocalMetricRetrieve (int mid, MetricResourceId *resource,
			 MetricValue **mv, int *num, 
			 time_t from, time_t to, int maxnum);
int LocalMetricRelease (MetricValue *mv);
int LocalMetricRegisterCallback (MetricCallback *mcb, int mid, int state);

static MetricRepositoryIF mrep = {
  "LocalRepository",
  LocalMetricAdd,
  LocalMetricRetrieve,
  LocalMetricRelease,
  LocalMetricRegisterCallback,
  LocalMetricResources,
  LocalMetricFreeResources,
};

MetricRepositoryIF *MetricRepository = &mrep;

MRWLOCK_DEFINE(LocalRepLock);

/*
 * the local repository consists of linked lists of values 
 * which are assumed to be ordered by timestamp
 */

typedef struct _MReposCallback {
  MetricCallback         *mrc_callback;
  struct _MReposCallback *mrc_next;
  int                     mrc_usecount;
} MReposCallback;

typedef struct _MReposValue {
  struct _MReposValue *mrv_next;
  MetricValue         *mrv_value;
} MReposValue;

struct _MReposHdr {
  int             mrh_id;
  MReposValue    *mrh_first;
  MReposCallback *mrh_cb;
} * LocalReposHeader = NULL;
size_t LocalReposNumIds = 0;


static int locateIndex(int mid);
static int addIndex(int mid);
static int resourceTest(MetricValue * actual, MetricResourceId * required);

static int pruneRepository();
static int _MetricRetrieveNoLock (int mid, MetricResourceId *resource,
				  MetricValue **mv, int *num, 
				  time_t from, time_t to, int maxnum);

int LocalMetricAdd (MetricValue *mv)
{
  int          idx;
  int          rc=-1;
  MReposValue *mrv;
  
  M_TRACE(MTRACE_FLOW,MTRACE_REPOS,
	  ("LocalMetricAdd %p (%d)", mv, mv?mv->mvId:-1));  
  /* obligatory prune check */
  pruneRepository();
  
  if (mv && MWriteLock(&LocalRepLock)==0) {  
    if (findres(mv->mvResource, mv->mvSystemId)==0) {
      M_TRACE(MTRACE_FLOW,MTRACE_REPOS,
	      ("LocalMetricAdd adding resource %s (%d)", 
	       mv->mvResource, mv->mvId));  
      addres(mv->mvResource, mv->mvSystemId);
    }
    idx=locateIndex(mv->mvId);
    if (idx==-1) {
      idx=addIndex(mv->mvId);
    }
    if (idx!=-1) {
      mrv=malloc(sizeof(MReposValue));
      mrv->mrv_next=LocalReposHeader[idx].mrh_first;
      mrv->mrv_value=malloc(sizeof(MetricValue));
      memcpy(mrv->mrv_value,mv,sizeof(MetricValue));
      mrv->mrv_value->mvResource=strdup(mv->mvResource);
      mrv->mrv_value->mvData=malloc(mv->mvDataLength);
      memcpy(mrv->mrv_value->mvData,mv->mvData,mv->mvDataLength);
      mrv->mrv_value->mvSystemId=strdup(mv->mvSystemId);
      LocalReposHeader[idx].mrh_first=mrv;
      if (LocalReposHeader[idx].mrh_cb) {
	MetricValue     *mvList;
	int              num;
	MetricResourceId mrId = {mv->mvResource,mv->mvSystemId};
	MReposCallback *cblist = LocalReposHeader[idx].mrh_cb;
	M_TRACE(MTRACE_FLOW,MTRACE_REPOS,
		("LocalMetricAdd processing callbacks for mid=%d", mv->mvId));
	/* It is not really expensive to call the retrieval function 
	   as the new value is at the list head.
	   Further, we don't bother whether it's a point or interval
	   metric: we always return the previous element, if available.
	 */
	rc = _MetricRetrieveNoLock(mv->mvId,&mrId,
				   &mvList,&num,0,mv->mvTimeStamp,2);
	while (rc == 0 && cblist && cblist->mrc_callback) {
	  cblist->mrc_callback(mvList,num);
	  cblist = cblist->mrc_next;
	}
	LocalMetricRelease(mvList);
      } 
      rc=0;
    }
    MWriteUnlock(&LocalRepLock);
  }
  return rc;
}

/*
 * Retrieves metric values for metric id mid,
 * a resource (if specified) in a given time range.
 * The MetricValue array must be freed by a call 
 * to LocalMetricRelease.
 *
 * Behavior:
 * I) maxnum <= 0
 *    i) to <= 0: retrieve everything starting with "from"
 *    ii) to > 0: retrieve everything between "from" and "to"
 * II) maxnum >0
 *    i) from >= to (!=0) retrieve "maxnum" entries starting with "from"  
 *    ii) from == 0 retrieve the newest "maxnum" entries 
 */

static int _MetricRetrieveNoLock (int mid, MetricResourceId *resource,
				  MetricValue **mv, int *num, 
				  time_t from, time_t to, int maxnum)
{
  int          idx;
  int          actnum=0;
  int          i;
  int          rc=-1;
  MReposValue *mrv = NULL;  
  MReposValue *first;
  idx=locateIndex(mid);
  if (idx!=-1) {
    if (to==0 || to ==-1) {
      to=LONG_MAX;
    }
    if (maxnum<=0) {
      maxnum=INT_MAX;
    }	else if (to==from) {
      /* maximum number requested: only "from" timestamp relevant */
      to=LONG_MAX;
    }
    mrv=LocalReposHeader[idx].mrh_first;
    first=NULL;
    *mv=NULL;
    while(mrv && from <= mrv->mrv_value->mvTimeStamp) {
      if (to >= mrv->mrv_value->mvTimeStamp && 
	  resourceTest(mrv->mrv_value,resource)==0) {
	/* mark */
	if (actnum < maxnum) {
	  if (first==NULL)
	    first=mrv;
	  actnum+=1;
	  if (from ==0 && actnum==maxnum)
	    break;
	} else {
	  if (first==NULL)
	    first=mrv;
	  else
	    first=first->mrv_next;
	}
      }
      mrv=mrv->mrv_next;
    }
    if (actnum) {
      *mv=calloc(actnum+1,sizeof(MetricValue));	
      for (i=0, mrv=first; i<actnum; mrv=mrv->mrv_next) {
	if (resourceTest(mrv->mrv_value,resource)==0) {
	  /* copy over */
	  memcpy(&(*mv)[i+1],mrv->mrv_value,sizeof(MetricValue));
	  (*mv)[i+1].mvData=malloc(mrv->mrv_value->mvDataLength);
	  memcpy((*mv)[i+1].mvData,
		 mrv->mrv_value->mvData,
		 mrv->mrv_value->mvDataLength);
	  (*mv)[i+1].mvResource=strdup(mrv->mrv_value->mvResource);
	  (*mv)[i+1].mvSystemId=strdup(mrv->mrv_value->mvSystemId);
	  i+=1;
	}
      }
    }
    if (*mv) {
      /* use the first entry to record array length */
      (*mv)[0].mvDataLength=actnum;
      (*mv)+=1;
      rc = 0;
    }
  }
  *num=actnum;
  return rc;
}

int LocalMetricRetrieve (int mid, MetricResourceId *resource,
			 MetricValue **mv, int *num, 
			 time_t from, time_t to, int maxnum)
{
  int          rc=-1;
  if (mv && num && from >=0 && MReadLock(&LocalRepLock)==0) {
    rc = _MetricRetrieveNoLock(mid,resource,mv,num,from,to,maxnum);
    MReadUnlock(&LocalRepLock);
  }
  return rc;
}

int LocalMetricRelease (MetricValue *mv)
{
  int i;
  if (mv) {
    mv-=1;
    for (i=0;i<mv->mvDataLength;i++) {
      free (mv[i+1].mvData);
      free (mv[i+1].mvResource);
      free (mv[i+1].mvSystemId);
    } 
    free(mv);
  }
  return 0;
}

int LocalMetricRegisterCallback (MetricCallback *mcb, int mid, int state)
{
  int idx = locateIndex(mid);
  MReposCallback * cblist;
  MReposCallback * cbprev;
  M_TRACE(MTRACE_FLOW,MTRACE_REPOS,("LocalMetricRegisterCallback=%p, %d, %d",mcb,mid,state));
  if (mcb == NULL || (state != 0 && state != 1) ) {
    M_TRACE(MTRACE_ERROR,MTRACE_REPOS,
	    ("LocalMetricRegisterCallback invalid parameter=%p, %d, %d",mcb,mid,state));
    return -1;
  } 
  if (idx == -1) {
    idx = addIndex(mid);
  }
  cblist = LocalReposHeader[idx].mrh_cb;
  cbprev = LocalReposHeader[idx].mrh_cb;
  if (state == MCB_STATE_REGISTER ) {
    while(cblist && cblist->mrc_callback) {
      if (mcb == cblist->mrc_callback) {
	break;
      }
      cblist = cblist->mrc_next;
    }
    if (cblist == NULL) {
      cblist = malloc(sizeof(MReposCallback));
      cblist->mrc_callback = mcb;
      cblist->mrc_next = LocalReposHeader[idx].mrh_cb;
      LocalReposHeader[idx].mrh_cb = cblist;
    }
    cblist->mrc_usecount ++;
  } else if ( state == MCB_STATE_UNREGISTER) {
    while(cblist && cblist->mrc_callback) {
      if (mcb == cblist->mrc_callback) {
	cblist->mrc_usecount--;
	break;
      }
      cbprev = cblist;
      cblist = cblist->mrc_next;
    }
    if (cblist && cblist->mrc_usecount == 0) {
      if (cbprev == LocalReposHeader[idx].mrh_cb) {
	LocalReposHeader[idx].mrh_cb = cblist->mrc_next;
      } else {
	cbprev->mrc_next=cblist->mrc_next;
      }
      free (cblist);
    }
  }
  return 0;
}

static int locateIndex(int mid)
{
  int idx=0;
  while (idx < LocalReposNumIds) {
    if (LocalReposHeader[idx].mrh_id == mid)
      return idx;
    else
      idx+=1;
  }
  return -1;
}

static int addIndex(int mid)
{
  LocalReposNumIds += 1;
  LocalReposHeader=realloc(LocalReposHeader,
			   LocalReposNumIds*sizeof(struct _MReposHdr));
  LocalReposHeader[LocalReposNumIds-1].mrh_cb=NULL;			   
  LocalReposHeader[LocalReposNumIds-1].mrh_first=NULL;			   
  LocalReposHeader[LocalReposNumIds-1].mrh_id=mid;			   
  return LocalReposNumIds-1;
}

static int resourceTest(MetricValue * actual, MetricResourceId * required)
{
  int teststate=-1;
  if (actual && actual->mvResource && actual->mvSystemId) {
    teststate=0;
    if (required->mrid_resource) {
      teststate=strcmp(actual->mvResource,required->mrid_resource);
    }
    if (teststate == 0 && required->mrid_system) {
      teststate=strcmp(actual->mvSystemId,required->mrid_system);
    }
  }
  return teststate;
}

#define PRUNE_INTERVAL      600
#define EXPIRATION_INTERVAL 900

static int pruneRepository()
{
  static time_t    nextPrune=0;
  size_t           numPruned=0;
  int              i;
  MReposValue      *v, *bv;
  time_t           now=time(NULL);

  if (nextPrune<now) {
    nextPrune = now + PRUNE_INTERVAL;
    if (MWriteLock(&LocalRepLock)==0) {
      for (i=0; i<LocalReposNumIds; i++) {
	bv=NULL;
	v = LocalReposHeader[i].mrh_first;
	while (v && v->mrv_value->mvTimeStamp > now - EXPIRATION_INTERVAL) {
	  bv = v;
	  v = v->mrv_next;
	}
	if (v && v->mrv_value->mvTimeStamp <= now - EXPIRATION_INTERVAL) {
	  if (bv==NULL)
	    LocalReposHeader[i].mrh_first = NULL;
	  else
	    bv->mrv_next = NULL;
	  /* now delete all outdated metrics*/
	  while (v) {
	    free(v->mrv_value->mvResource); 
	    free(v->mrv_value->mvData);
	    free(v->mrv_value->mvSystemId);
	    free(v->mrv_value);
	    bv=v;
	    v=v->mrv_next;
	    free(bv);
	    numPruned++;
	  }
	}
      }
      MWriteUnlock(&LocalRepLock);
    }
  }
  return numPruned;
}


/* TODO: away with that */
int findres(char *res, char *sys)
{
  size_t c;
  for (c=0; c<resnum;c++) {
    if (strcmp(reslist[c].mrid_resource,res)==0 && 
	strcmp(reslist[c].mrid_system,sys)==0 )
      return 1;
  }
  return 0;
}

void addres(char *res, char *sys)
{
  reslist = realloc(reslist,(resnum+1)*sizeof(MetricResourceId));
  reslist[resnum].mrid_resource=strdup(res);
  reslist[resnum].mrid_system=strdup(sys);
  resnum ++;
}

int LocalMetricResources(int id, MetricResourceId ** resources,
			 const char * resource, const char * system)
{
  int i;
  int num = 0;
  int state;

  if (resource==NULL && system==NULL) {
    *resources = reslist;
    return resnum;
  } else {
    *resources = NULL;
    for (i=0; i < resnum; i++) {
      state = 0;
      if (resource && strcmp(reslist[i].mrid_resource,resource)) {
	continue;
      }
      if (system && strcmp(reslist[i].mrid_system,system)) {
	continue;
      }
      num ++;
      *resources=realloc(*resources,sizeof(MetricResourceId)*(num+1));
      (*resources)[num-1].mrid_resource=strdup(reslist[i].mrid_resource);
      (*resources)[num-1].mrid_system=strdup(reslist[i].mrid_system);
      (*resources)[num].mrid_resource=NULL;
    }
    return num;
  }
}

int LocalMetricFreeResources(MetricResourceId * resources)
{
  int i=0;
  if (resources && resources != reslist) {
    /* must free temporary resource list */
    while (resources[i].mrid_resource) {
      free (resources[i].mrid_resource);
      free (resources[i].mrid_system);
    }
    free (resources);
  }
  return 0;
}
