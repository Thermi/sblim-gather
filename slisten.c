/*
 * $Id: slisten.c,v 1.2 2004/11/18 15:53:32 mihajlov Exp $
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
 * Description: Subscription Forwarding
 * 
 */

#include "slisten.h"
#include "marshal.h"
#include "mtrace.h"
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKFILE_TEMPLATE "/tmp/rrepos-socket-XXXXXX"


static pthread_t pt_listener;
static char listener[] = SOCKFILE_TEMPLATE;
static int  fdsockfile = -1;

typedef struct _CallbackEntry {
  int                    cb_metricid;
  int                    cb_corrid;
  SubscriptionCallback * cb_callback;
  struct _CallbackEntry *cb_next;
} CallbackEntry;

static CallbackEntry * cbHead = NULL;

static void call_callbacks(int corrid, ValueRequest *vr)
{
  CallbackEntry * cbl = cbHead;
  while(cbl && corrid <= cbl->cb_corrid) {
    if (cbl->cb_corrid == corrid && cbl->cb_metricid == vr->vsId ) {
      cbl->cb_callback(corrid,vr);
    }
    cbl = cbl->cb_next;
  }
}

static void subs_listener_exit(void)
{
  /* avoids socket file ruins in /tmp */
  unlink(listener);
}

static void subs_listener_cleanup(void *fdsocket)
{
  return;
  /* reset to initial state */
  close((int)fdsocket);
  unlink(listener);
  strcpy(listener,SOCKFILE_TEMPLATE);
  fdsockfile=-1;
}

static void * subs_listener(void *unused)
{
  int                fdsocket;
  struct sockaddr_un sockname;
  int               *corrid;
  ValueRequest      *vr;
  char               buf[1000];
  off_t              offset;

  M_TRACE(MTRACE_FLOW,MTRACE_RREPOS,
	  ("subs_listener"));
  fdsocket = socket(AF_UNIX,SOCK_DGRAM,0);
  pthread_cleanup_push(subs_listener_cleanup,(void*)fdsocket);
  if (fdsocket != -1) {
    atexit(subs_listener_exit);
    sockname.sun_family = AF_UNIX;
    strcpy(sockname.sun_path,listener);
    if (bind(fdsocket,(struct sockaddr*)&sockname,sizeof(sockname))==-1) {
      M_TRACE(MTRACE_ERROR,MTRACE_RREPOS,
	      ("subs_listener bind error (%s: %s)",strerror(errno),
	       sockname.sun_path));
    }
    while(1) {
      offset=0;
      if (recv(fdsocket,buf,sizeof(buf),0) != -1) {
	if (unmarshal_data((void**)&corrid,sizeof(int),
			   buf,&offset,sizeof(buf)) == 0 &&
	    unmarshal_valuerequest(&vr,buf,&offset,
				   sizeof(buf)) == 0) {
	  call_callbacks(*corrid,vr);
	} else {
	  M_TRACE(MTRACE_ERROR,MTRACE_RREPOS,
		  ("subs_listener unmmarshalling error"));
	}	  
      } else {
	if (errno==EINTR) {
	  M_TRACE(MTRACE_ERROR,MTRACE_RREPOS,
		  ("subs_listener interrupted"));
	  continue;
	}
	M_TRACE(MTRACE_ERROR,MTRACE_RREPOS,
		("subs_listener recv error (%s)",strerror(errno)));
	break;
      }
    }
    /* clean up after an error occurs */
  } else {
    M_TRACE(MTRACE_ERROR,MTRACE_RREPOS,
	    ("subs_listener socket error (%s)",strerror(errno)));
    /* socket error */
  }

  pthread_cleanup_pop(1);
  return NULL;
}

int add_subscription_listener(char *listenerid, SubscriptionRequest *sr,
			      SubscriptionCallback *scb)
{
  CallbackEntry *cbl = cbHead;
  CallbackEntry *prev = cbHead;
  M_TRACE(MTRACE_FLOW,MTRACE_RREPOS,
	  ("setup_subscription_listener"));
  if (fdsockfile==-1) {
    fdsockfile=mkstemp(listener);
    M_TRACE(MTRACE_DETAILED,MTRACE_RREPOS,
	    ("listener socket name = %s",listener));
    if (fdsockfile != -1) {
      close(fdsockfile);
      unlink(listener);
      pthread_create(&pt_listener,NULL,subs_listener,NULL);
      pthread_detach(pt_listener);
    } else {
      M_TRACE(MTRACE_ERROR,MTRACE_RREPOS,
	      ("could not create listener socket file %s",listener));
      return -1;
    }
  }
  strcpy(listenerid,listener);
  while (cbl) {
    if (cbl->cb_metricid == sr->srMetricId && 
	cbl->cb_corrid == sr->srCorrelatorId &&
	cbl->cb_callback == scb) {
      /* already there */
      M_TRACE(MTRACE_DETAILED,MTRACE_REPOS,
	      ("already enabled metric id %d for listening", sr->srMetricId));
      return 0;
    } else if (cbl->cb_next && cbl->cb_next->cb_corrid > sr->srCorrelatorId) {
      break;
    }
    prev = cbl;
    cbl = cbl->cb_next;
  }
  M_TRACE(MTRACE_DETAILED,MTRACE_REPOS,
	  ("enabling metric id %d for listening", sr->srMetricId));
  cbl = malloc(sizeof(CallbackEntry));
  cbl->cb_metricid = sr->srMetricId;
  cbl->cb_corrid = sr->srCorrelatorId;
  cbl->cb_callback = scb;
  if (cbHead == prev && 
      (cbHead == NULL || cbHead->cb_corrid > sr->srCorrelatorId) ) {
    cbl->cb_next = cbHead;
    cbHead = cbl;
  } else {
    cbl->cb_next = prev->cb_next;
    prev->cb_next = cbl;
  }
  return 0;
}
