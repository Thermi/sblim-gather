/*
 * $Id: mreposl.c,v 1.4 2004/08/04 07:29:26 mihajlov Exp $
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
 * Description: Local Metric Value Repository
 * This is an implementation of a history-less (well it caches 15 minutes)
 * and local repository. Others will follow.
 */

#include "mrepos.h"
#include "mrwlock.h"

#include "mreg.h" 

#include <stdlib.h>
#include <limits.h>
#include <string.h>


/* TODO: must revise repository IF to not require resource listing */

static char ** reslist = NULL;
static size_t  resnum = 0;
static int findres(char *);
static void addres(char *);

int LocalMetricResources(int id, char *** resources);
int LocalMetricFreeResources(char ** resources);
  

/**/

int LocalMetricAdd (MetricValue *mv);
int LocalMetricRetrieve (int mid, char *resource,
			 MetricValue **mv, int *num, 
			 time_t from, time_t to, int maxnum);
int LocalMetricRelease (MetricValue *mv);

static MetricRepositoryIF mrep = {
  "LocalRepository",
  LocalMetricAdd,
  LocalMetricRetrieve,
  LocalMetricRelease,
  LocalMetricResources,
  LocalMetricFreeResources,
};

MetricRepositoryIF *MetricRepository = &mrep;

MRWLOCK_DEFINE(LocalRepLock);

/*
 * the local repository consists of linked lists of values 
 * which are assumed to be ordered by timestamp
 */

typedef struct _MReposValue {
  struct _MReposValue *mrv_next;
  MetricValue         *mrv_value;
} MReposValue;

struct _MReposHdr {
  int          mrh_id;
  MReposValue *mrh_first;
} * LocalReposHeader = NULL;
size_t LocalReposNumIds = 0;


static int locateIndex(int mid);
static int addIndex(int mid);
static int resourceTest(const char * actual, const char * required);

static int pruneRepository();

int LocalMetricAdd (MetricValue *mv)
{
  int          idx;
  int          rc=-1;
  MReposValue *mrv;
  
  /* obligatory prune check */
  pruneRepository();
  
  if (mv && MWriteLock(&LocalRepLock)==0) {  
    if (findres(mv->mvResource)==0) addres(mv->mvResource);
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
      LocalReposHeader[idx].mrh_first=mrv;
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

int LocalMetricRetrieve (int mid, char *resource,
			 MetricValue **mv, int *num, 
			 time_t from, time_t to, int maxnum)
{
  int          idx;
  int          actnum=0;
  int          i;
  int          rc=-1;
  MReposValue *mrv = NULL;  
  MReposValue *first;
  if (mv && num && from >=0 && MReadLock(&LocalRepLock)==0) {
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
	    resourceTest(mrv->mrv_value->mvResource,resource)==0) {
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
	  if (resourceTest(mrv->mrv_value->mvResource,resource)==0) {
	    /* copy over */
	    memcpy(&(*mv)[i+1],mrv->mrv_value,sizeof(MetricValue));
	    (*mv)[i+1].mvData=malloc(mrv->mrv_value->mvDataLength);
	    memcpy((*mv)[i+1].mvData,
		   mrv->mrv_value->mvData,
		   mrv->mrv_value->mvDataLength);
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
    MReadUnlock(&LocalRepLock);
  }
  *num=actnum;
  return rc;
}

int LocalMetricRelease (MetricValue *mv)
{
  int i;
  if (mv) {
    mv-=1;
    for (i=0;i<mv->mvDataLength;i++) {
      free (mv[i+1].mvData);
    } 
    free(mv);
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
  LocalReposHeader[LocalReposNumIds-1].mrh_first=NULL;			   
  LocalReposHeader[LocalReposNumIds-1].mrh_id=mid;			   
  return LocalReposNumIds-1;
}

static int resourceTest(const char * actual, const char * required)
{
  if (actual) {
    if (required) {
      return strcmp(actual,required);
    }
    return 0;
  }
  return -1;
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
	    if (v->mrv_value->mvResource) free (v->mrv_value->mvResource); 
	    free(v->mrv_value->mvData);
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
int findres(char *res)
{
  size_t c;
  for (c=0; c<resnum;c++) {
    if (strcmp(reslist[c],res)==0)
      return 1;
  }
  return 0;
}

void addres(char *res)
{
  reslist = realloc(reslist,(resnum+1)*sizeof(char*));
  reslist[resnum++]=strdup(res);
}

int LocalMetricResources(int id, char *** resources)
{
  *resources = reslist;
  return resnum;
}

int LocalMetricFreeResources(char ** resources)
{
  return 0;
}
