/*
 * $Id: reposd.c,v 1.22 2004/12/22 15:43:36 mihajlov Exp $
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
 * Contributors: Michael Schuele <schuelem@de.ibm.com>
 *
 * Description: Repository Daemon
 *
 * The Performance Data Repository Daemon is receiving and storing 
 * metric information from a Gatherer daemon
 * and returning request results and status information to clients.
 * 
 */

#include "repos.h"
#include "sforward.h"
#include "gatherc.h"
#include "commheap.h"
#include "marshal.h"
#include "reposcfg.h"
#include <mtrace.h>
#include <mlog.h>
#include <mcserv.h>
#include <rcserv.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <memory.h>
#include <pthread.h>

#define CHECKBUFFER(comm,buffer,sz) ((comm)->gc_datalen+sizeof(GATHERCOMM)+(sz)<=sizeof(buffer))

/* ---------------------------------------------------------------------------*/

static char rreposport_s[10] = {0};
static int  rreposport       = 0;
static char rmaxconn_s[10]   = {0};
static int  rmaxconn         = 0;
static int  connects         = 0;
static pthread_mutex_t connect_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  connect_cond  = PTHREAD_COND_INITIALIZER;

static void * repos_remote();
static void * rrepos_getrequest(void * hdl);

#define RINITCHECK() \
pthread_mutex_lock(&connect_mutex); \
if (rreposport==0) { \
  if (reposcfg_getitem("RepositoryPort",rreposport_s,sizeof(rreposport_s)) == 0) {  \
    rreposport=atoi(rreposport_s); \
  } else { \
    rreposport=6363; \
  } \
  if (reposcfg_getitem("RepositoryMaxConnections",rmaxconn_s,sizeof(rmaxconn_s)) == 0) {  \
    rmaxconn=atoi(rmaxconn_s); \
  } else { \
    rmaxconn=100; \
  } \
  rcs_init(&rreposport); \
} \
pthread_mutex_unlock(&connect_mutex); 


/* ---------------------------------------------------------------------------*/

int main(int argc, char * argv[])
{
  int           quit=0;
  MC_REQHDR     hdr;
  GATHERCOMM   *comm;
  COMMHEAP     *ch;
  char          buffer[GATHERVALBUFLEN];
  size_t        bufferlen=sizeof(buffer);
  SubscriptionRequest *sr;
  ValueRequest *vr;
  ValueItem    *vi;
  void         *vp;
  char         *vpmax;
  int           i;
  size_t        valreslen;
  size_t        valsyslen;
  off_t         offset;
  RepositoryPluginDefinition *rdef;
  char         *pluginname, *metricname, *listener;
  MetricResourceId *rid;
  MetricValue  *mv;
  pthread_t     rcomm;
  char          cfgbuf[1000];
  char         *cfgidx1, *cfgidx2;

  m_start_logging("reposd");
  m_log(M_INFO,M_QUIET,"Reposd is starting up.\n");

  if (argc == 1) {
    /* daemonize if no arguments are given */
    if (daemon(0,0)) {
      m_log(M_ERROR,M_SHOW,"Couldn't daemonize: %s - exiting\n",
	    strerror(errno));
      exit(-1);
    }
  }
  
  if (reposcfg_init()) {
    m_log(M_ERROR,M_SHOW,"Could not open reposd config file.\n");
  }
  
  /* init tracing from config */
#ifndef NOTRACE
  if (reposcfg_getitem("TraceLevel",cfgbuf,sizeof(cfgbuf))==0) {
    m_trace_setlevel(atoi(cfgbuf));
  }
  if (reposcfg_getitem("TraceFile",cfgbuf,sizeof(cfgbuf)) ==0) {
	m_trace_setfile(cfgbuf);
  }
  if (reposcfg_getitem("TraceComponents",cfgbuf,sizeof(cfgbuf)) ==0) {
    cfgidx1 = cfgbuf;
    while (cfgidx1) {
      cfgidx2 = strchr(cfgidx1,':');
      if (cfgidx2) {
	*cfgidx2++=0;
      }
      m_trace_enable(m_trace_compid(cfgidx1));
      cfgidx1=cfgidx2;
    }
  }
#endif
  
  M_TRACE(MTRACE_DETAILED,MTRACE_REPOS,("Reposd tracing initialized."));

  if (mcs_init(REPOS_COMMID)) {
    m_log(M_ERROR,M_SHOW,"Could not open reposd socket.\n");
    return -1;
  }

  /* start remote reposd in a separate thread */
  RINITCHECK();
  if (pthread_create(&rcomm,NULL,repos_remote,NULL) != 0) {
    m_log(M_ERROR,M_SHOW,"Could not create remote reposd thread: %s.\n",
	  strerror(errno));
  }

  memset(buffer, 0, sizeof(buffer));
 
  while (!quit && mcs_accept(&hdr)==0) {
    while (!quit && mcs_getrequest(&hdr, buffer, &bufferlen)==0) {
      M_TRACE(MTRACE_FLOW,MTRACE_REPOS,("Received message type=%d",hdr.mc_type));
      if (hdr.mc_type != GATHERMC_REQ) {
	/* ignore unknown message types */
	m_log(M_ERROR,M_QUIET,"Invalid request type  received %c\n",hdr.mc_type);
	continue;
      }
      comm=(GATHERCOMM*)buffer;
      /* perform sanity check */
      if (bufferlen != sizeof(GATHERCOMM) + comm->gc_datalen) {
	m_log(M_ERROR,M_QUIET,"Invalid length received: expected %d got %d\n",
	      sizeof(GATHERCOMM)+comm->gc_datalen,bufferlen);
	continue;
      }
      M_TRACE(MTRACE_FLOW,MTRACE_REPOS,("Received command id=%d",comm->gc_cmd));
      switch (comm->gc_cmd) {
      case GCMD_STATUS:
	repos_status((RepositoryStatus*)(buffer+sizeof(GATHERCOMM)));
	comm->gc_result=0;
	comm->gc_datalen=sizeof(RepositoryStatus);
	break;
      case GCMD_INIT:
	comm->gc_result=repos_init();
	comm->gc_datalen=0;
	break;
      case GCMD_TERM:
	comm->gc_result=repos_terminate();
	comm->gc_datalen=0;
	break;
      case GCMD_GETTOKEN:
	comm->gc_result=
	  repos_sessiontoken((RepositoryToken*)(buffer+sizeof(GATHERCOMM)));
	comm->gc_datalen=sizeof(RepositoryToken);
	break;
      case GCMD_ADDPLUGIN:
	offset = sizeof(GATHERCOMM);
	if (unmarshal_string(&pluginname,buffer,&offset,sizeof(buffer),1) == 0) {
	  comm->gc_result=reposplugin_add(pluginname);
	} else {
	  M_TRACE(MTRACE_ERROR,MTRACE_REPOS,
		  ("Unmarshalling add plugin request failed %d/%d.",
		   offset,sizeof(buffer)));
	  m_log(M_ERROR,M_QUIET,
		"Unmarshalling add plugin request failed %d/%d.",
		offset,sizeof(buffer));
	  comm->gc_result=-1;
	}
	if (comm->gc_result != 0) {
	  M_TRACE(MTRACE_ERROR,MTRACE_REPOS,
		  ("Could not load repository plugin %s.",
		   pluginname));
	  m_log(M_ERROR,M_QUIET,
		"Could not load repository plugin %s.",
		pluginname);		
	}
	comm->gc_datalen=0;
	break;
      case GCMD_REMPLUGIN:
	offset = sizeof(GATHERCOMM);
	if (unmarshal_string(&pluginname,buffer,&offset,sizeof(buffer),1) == 0) {
	  comm->gc_result=reposplugin_remove(pluginname);
	} else {
	  M_TRACE(MTRACE_ERROR,MTRACE_REPOS,
		  ("Unmarshalling remove plugin request failed %d/%d.",
		   offset,sizeof(buffer)));
	  m_log(M_ERROR,M_QUIET,
		"Unmarshalling remove plugin request failed %d/%d.",
		offset,sizeof(buffer));
	  comm->gc_result=-1;
	}
	if (comm->gc_result != 0) {
	  M_TRACE(MTRACE_ERROR,MTRACE_REPOS,
		  ("Could not unload repository plugin %s.",
		   pluginname));
	  m_log(M_ERROR,M_QUIET,
		"Could not unload repository plugin %s.",
		pluginname);		
	}
	comm->gc_datalen=0;
	break;
      case GCMD_LISTPLUGIN:
	offset = sizeof(GATHERCOMM);
	ch=ch_init();
	if (unmarshal_string(&pluginname,buffer,&offset,sizeof(buffer),1) == 0) {
	  comm->gc_result=reposplugin_list(pluginname,
					   &rdef,
					   ch);
	  if (comm->gc_result > 0) {
	    if (marshal_reposplugindefinition(rdef,comm->gc_result,buffer,
					      &offset,sizeof(buffer))) {
	      M_TRACE(MTRACE_ERROR,MTRACE_REPOS,
		      ("Marshalling list plugin response failed %d/%d.",
		       offset,sizeof(buffer)));
	      m_log(M_ERROR,M_QUIET,
		    "Marshalling list plugin response failed %d/%d.",
		    offset,sizeof(buffer));
	      comm->gc_result=-1;
	    }
	  }
	} else {
	  M_TRACE(MTRACE_ERROR,MTRACE_REPOS,
		  ("Unmarshalling list plugin request failed %d/%d.",
		   offset,sizeof(buffer)));
	  m_log(M_ERROR,M_QUIET,
		"Unmarshalling list plugin request failed %d/%d.",
		 offset,sizeof(buffer));
	  comm->gc_result=-1;
	}
	if (comm->gc_result == -1) {
	  M_TRACE(MTRACE_ERROR,MTRACE_REPOS,
		  ("Could not list repository plugin %s definitions.",
		   pluginname));
	  m_log(M_ERROR,M_QUIET,
		"Could not list repository plugin %s definitions.",
		pluginname);		
	}
	comm->gc_datalen=offset;
	ch_release(ch);
	break;
      case GCMD_LISTRESOURCES:
	ch=ch_init();
	comm->gc_result=reposresource_list(buffer+sizeof(GATHERCOMM),
					   &rid,
					   ch);
	if (comm->gc_result > 0) {
	  if (CHECKBUFFER(comm,buffer,strlen(buffer+sizeof(GATHERCOMM))+ 1 +
			  comm->gc_result*sizeof(MetricResourceId))) {
	    comm->gc_datalen=strlen(buffer+sizeof(GATHERCOMM))+ 1 +
	      comm->gc_result*sizeof(MetricResourceId);
	    memcpy(buffer+sizeof(GATHERCOMM)+strlen(buffer+sizeof(GATHERCOMM))+1,
		   rid,
		   comm->gc_result*sizeof(MetricResourceId));
	    for (i=0; i < comm->gc_result; i++) {
	      if (!CHECKBUFFER(comm,buffer,strlen(rid[i].mrid_resource) + 1)) {
		comm->gc_result=-1;
		break;
	      }
	      memcpy(buffer+sizeof(GATHERCOMM)+comm->gc_datalen,
		     rid[i].mrid_resource,
		     strlen(rid[i].mrid_resource) + 1);
	      comm->gc_datalen += strlen(rid[i].mrid_resource) + 1;
	      if (!CHECKBUFFER(comm,buffer,strlen(rid[i].mrid_system) + 1)) {
		comm->gc_result=-1;
		break;
	      }
	      memcpy(buffer+sizeof(GATHERCOMM)+comm->gc_datalen,
		     rid[i].mrid_system,
		     strlen(rid[i].mrid_system) + 1);
	      comm->gc_datalen += strlen(rid[i].mrid_system) + 1;
	    }
	  } else {
	    comm->gc_result=-1;
	  }
	} else {
	  comm->gc_datalen=strlen(buffer+sizeof(GATHERCOMM))+ 1;
	}
	ch_release(ch);
	break;
      case GCMD_SUBSCRIBE:
	offset = sizeof(GATHERCOMM);
	if (unmarshal_string(&listener,buffer,&offset,sizeof(buffer),1) == 0 &&
	    unmarshal_subscriptionrequest(&sr,buffer,&offset,sizeof(buffer)) 
	    == 0) {
	  if (subs_enable_forwarding(sr,listener)==0) {
	    comm->gc_result=repos_subscribe(sr,subs_forward);
	  } else {
	    M_TRACE(MTRACE_ERROR,MTRACE_REPOS,
		    ("Enabling of susbcription forwarding failed."));
	    comm->gc_result=-1;
	  }
	} else {
	  M_TRACE(MTRACE_ERROR,MTRACE_REPOS,
		  ("Unmarshalling subscription request failed %d/%d.",
		   offset,sizeof(buffer)));
	  comm->gc_result=-1;
	}
	comm->gc_datalen=0;
	break;
      case GCMD_UNSUBSCRIBE:
	offset = sizeof(GATHERCOMM);
	if (unmarshal_string(&listener,buffer,&offset,sizeof(buffer),1) == 0 &&
	    unmarshal_subscriptionrequest(&sr,buffer,&offset,sizeof(buffer)) 
	    == 0) {
	  if (subs_disable_forwarding(sr,listener)==0) {
	    comm->gc_result=repos_unsubscribe(sr,subs_forward);
	  } else {
	    M_TRACE(MTRACE_ERROR,MTRACE_REPOS,
		    ("Disabling of susbcription forwarding failed."));
	    comm->gc_result=-1;
	  }
	} else {
	  M_TRACE(MTRACE_ERROR,MTRACE_REPOS,
		  ("Unmarshalling unsubscription request failed %d/%d.",
		   offset,sizeof(buffer)));
	  comm->gc_result=-1;
	}
	comm->gc_datalen=0;
	break;
      case GCMD_SETVALUE:
	/* the transmitted parameters are
	 * 1: pluginname
	 * 2: metricname
	 * 3: metricvalue
	 */
	pluginname=buffer+sizeof(GATHERCOMM);
	metricname=pluginname+strlen(pluginname)+1;
	mv=(MetricValue*)(metricname+strlen(metricname)+1);
	/* fixups */
	if (mv->mvResource) {
	  mv->mvResource=(char*)(mv + 1);
	  mv->mvData=mv->mvResource + strlen(mv->mvResource) + 1;
	} else {
	  mv->mvData=(char*)(mv + 1);
	}
	mv->mvSystemId=mv->mvData+mv->mvDataLength;
	comm->gc_result=reposvalue_put(pluginname,metricname,mv);
	comm->gc_datalen=0;
	break;      
      case GCMD_GETVALUE:
	/* the transmitted ValueRequest contains
	 * 1: Header
	 * 2: ResourceNresourceame
	 * 3: System Id
	 * 4: ValueItems
	 */
	vr=(ValueRequest *)(buffer+sizeof(GATHERCOMM));
	vp=vr;
	vpmax=(char*)buffer+sizeof(buffer);
	if (vr->vsResource) {
	  /* adjust pointer to resource name */
	  vr->vsResource = (char*)vr + sizeof(ValueRequest);
	  valreslen= strlen(vr->vsResource) + 1;
	} else {
	  valreslen = 0;
	}
	if (vr->vsSystemId) {
	  /* adjust pointer to system id */
	  vr->vsSystemId = (char*)vr + sizeof(ValueRequest) + valreslen;
	  valsyslen = strlen(vr->vsSystemId) + 1;
	} else {
	  valsyslen = 0;
	}
	vi=(ValueItem*)((char*)vr+sizeof(ValueRequest)+valreslen+valsyslen);
	vr->vsValues=NULL;
	ch=ch_init();
	
	comm->gc_result=reposvalue_get(vr,ch);
	/* copy value data into transfer buffer and compute total length */
	if (comm->gc_result != -1) {
	  if ((char*)vi+sizeof(ValueItem)*vr->vsNumValues < vpmax) {
	    memcpy(vi,vr->vsValues,sizeof(ValueItem)*vr->vsNumValues);
	    vp = (char*)vi + sizeof(ValueItem) * vr->vsNumValues;
	    for (i=0;i<vr->vsNumValues;i++) {
	      M_TRACE(MTRACE_FLOW,MTRACE_REPOS,
		      ("Returning value for mid=%d, resource %s: %d",
		       vr->vsId,
		       vr->vsValues[i].viResource,
		       *(int*)vr->vsValues[i].viValue));
	      if ((char*)vp + vr->vsValues[i].viValueLen < vpmax) {
		memcpy(vp,vr->vsValues[i].viValue,vr->vsValues[i].viValueLen);
		vi[i].viValue = vp;
		vp = (char*)vp + vi[i].viValueLen;
		if (vr->vsValues[i].viResource) {
		  if ((char*)vp + strlen(vr->vsValues[i].viResource) + 1 < vpmax) {
		    strcpy(vp,vr->vsValues[i].viResource);
		    vi[i].viResource = vp;
		    vp = (char*)vp + strlen(vp)+1;
		  }
		} else {
		  comm->gc_result=-1;
		  break;
		}
		if (vr->vsValues[i].viSystemId) {
		  if ((char*)vp + strlen(vr->vsValues[i].viSystemId) + 1 < vpmax) {
		    strcpy(vp,vr->vsValues[i].viSystemId);
		    vi[i].viSystemId = vp;
		    vp = (char*)vp + strlen(vp)+1;
		  }
		} else {
		  comm->gc_result=-1;
		  break;
		}
	      } else {
		comm->gc_result=-1;
		break;
	      }
	    }
	  } else {
	    comm->gc_result=-1;
	  }
	  comm->gc_datalen= (char*)vp - (char*)vr;
	}
	ch_release(ch);
	break;
      case GCMD_QUIT:
	quit=1;
	comm->gc_result=0;
	comm->gc_datalen=0;
	break;
      default:
	comm->gc_result=-1;
	comm->gc_datalen=0;
	break;
      }
      hdr.mc_type=GATHERMC_RESP;
      if (sizeof(GATHERCOMM) + comm->gc_datalen > sizeof(buffer)) {
	m_log(M_ERROR,M_QUIET,
	      "Error: Available data size is exceeding buffer.\n");
      }
      mcs_sendresponse(&hdr,buffer,sizeof(GATHERCOMM)+comm->gc_datalen);
      bufferlen=sizeof(buffer);
    }
  }
  mcs_term();
  m_log(M_INFO,M_QUIET,"Reposd is shutting down.\n");
  M_TRACE(MTRACE_FLOW,MTRACE_REPOS,("Reposd is shutting down."));
  return 0;
}

static void * repos_remote()
{
  pthread_t thread;
  int       hdl = -1;
  
  m_log(M_INFO,M_QUIET,"Remote reposd is starting up.\n");
  M_TRACE(MTRACE_FLOW,MTRACE_REPOS,("Remote reposd is starting up."));

  while (1) {
    if (hdl == -1) {
      if (rcs_accept(&hdl) == -1) {
	m_log(M_ERROR,M_SHOW,"Remote reposd could not accept: %s.\n",
	      strerror(errno));
	return 0; 
      }
      M_TRACE(MTRACE_FLOW,MTRACE_REPOS,("Remote client request on socket %i accepted",hdl));
    }
    pthread_mutex_lock(&connect_mutex);
    connects++;
    M_TRACE(MTRACE_FLOW,MTRACE_REPOS,
	    ("Increased number of current remote client connections %i",connects));
    pthread_mutex_unlock(&connect_mutex);
    if (pthread_create(&thread,NULL,rrepos_getrequest,(void*)hdl) != 0) {
      m_log(M_ERROR,M_SHOW,"Remote reposd could not create thread for socket %i: %s.\n",
	    hdl,strerror(errno));
      return 0;
    }   
    pthread_mutex_lock(&connect_mutex);
    if(connects==rmaxconn) {
      /* wait for at least one finished thread */
      M_TRACE(MTRACE_FLOW,MTRACE_REPOS,
	      ("Remote reposd's MaxConnections reached %i - waiting for at least one finished thread",
	       connects));
      pthread_cond_wait(&connect_cond,&connect_mutex);
    }
    pthread_mutex_unlock(&connect_mutex);
    hdl = -1;
  }
  rcs_term();
  m_log(M_INFO,M_QUIET,"Remote reposd is shutting down.\n");
  M_TRACE(MTRACE_FLOW,MTRACE_REPOS,("Remote reposd is shutting down."));
  return 0;
}

static void * rrepos_getrequest(void * hdl)
{
  GATHERCOMM   *comm;
  char *buffer = malloc(GATHERVALBUFLEN);
;
  size_t        bufferlen;
  char         *pluginname, *metricname;
  MetricValue  *mv;

  M_TRACE(MTRACE_FLOW,MTRACE_REPOS,("Starting thread on socket %i",(int)hdl));
  if (pthread_detach(pthread_self()) != 0) {
    m_log(M_ERROR,M_SHOW,"Remote reposd thread could not detach: %s.\n",
	  strerror(errno));
  }
  M_TRACE(MTRACE_FLOW,MTRACE_REPOS,("Detached thread on socket %i",(int)hdl));

  while (1) {
    memset(buffer, 0, GATHERVALBUFLEN);
    bufferlen=GATHERVALBUFLEN;

    pthread_mutex_lock(&connect_mutex);

    if (rcs_getrequest((int)hdl,buffer,&bufferlen) == -1) {
      connects--;
      M_TRACE(MTRACE_FLOW,MTRACE_REPOS,
	    ("Decreased number of current remote client connections %i",connects));
      pthread_cond_signal(&connect_cond);
      pthread_mutex_unlock(&connect_mutex);
      break;
    }
    pthread_mutex_unlock(&connect_mutex);

    /* write data to repository */
    comm=(GATHERCOMM*)buffer;    
    comm->gc_cmd     = ntohs(comm->gc_cmd);
    comm->gc_datalen = ntohl(comm->gc_datalen);
    comm->gc_result  = ntohs(comm->gc_result);
        
    /* perform sanity check */
    if (bufferlen != sizeof(GATHERCOMM) + comm->gc_datalen) {
      m_log(M_ERROR,M_SHOW,
	    "Remote reposd invalid length received on socket %i: expected %d got %d.\n",
	    hdl,sizeof(GATHERCOMM)+comm->gc_datalen,bufferlen);
      continue;
    }
    /* the transmitted parameters are
     * 1: pluginname
     * 2: metricname
     * 3: metricvalue
     */
    pluginname=buffer+sizeof(GATHERCOMM);
    metricname=pluginname+strlen(pluginname)+1;
    mv=(MetricValue*)(metricname+strlen(metricname)+1);

    /* fixups */
    if (mv->mvResource) {
      mv->mvResource=(char*)(mv + 1);
      mv->mvData=mv->mvResource + strlen(mv->mvResource) + 1;
    } else {
      mv->mvData=(char*)(mv + 1);
    }

    /* convert from host byte order into network byte order */
    mv->mvId         = ntohl((int)mv->mvId);
    mv->mvTimeStamp  = ntohl((time_t)mv->mvTimeStamp);
    mv->mvDataType   = ntohl((unsigned)mv->mvDataType);
    mv->mvDataLength = ntohl((size_t)mv->mvDataLength);
    mv->mvSystemId=mv->mvData+mv->mvDataLength;
    M_TRACE(MTRACE_FLOW,MTRACE_REPOS,
	    ("Retrieved data on socket %i: %s %s %s",(int)hdl,
	     mv->mvSystemId,pluginname,metricname));
    if ((comm->gc_result=reposvalue_put(pluginname,metricname,mv)) != 0) {
      m_log(M_ERROR,M_SHOW,"Remote reposd on socket %i: write %s to repository failed.\n",
	    hdl,metricname);
    }
  }
  M_TRACE(MTRACE_FLOW,MTRACE_REPOS,("Ending thread on socket %i",(int)hdl));
  free(buffer);
  return NULL;
}
