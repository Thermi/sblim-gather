/*
 * $Id: reposd.c,v 1.12 2004/10/20 14:43:31 heidineu Exp $
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
 * Description: Repository Daemon
 *
 * The Performance Data Repository Daemon is receiving and storing 
 * metric information from a Gatherer daemon
 * and returning request results and status information to clients.
 * 
 */

#include "repos.h"
#include "gatherc.h"
#include "commheap.h"
#include "mlog.h"
#include "reposcfg.h"
#include <mcserv.h>
#include <rcserv.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>

#define CHECKBUFFER(comm,buffer,sz) ((comm)->gc_datalen+sizeof(GATHERCOMM)+(sz)<=sizeof(buffer))

/* ---------------------------------------------------------------------------*/

static char rreposport_s[10] = {0};
static int  rreposport       = 0;
static char rmaxconn_s[10]   = {0};
static int  rmaxconn         = 0;
static int  connects         = 0;
static pthread_mutex_t connect_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  connect_cond  = PTHREAD_COND_INITIALIZER;

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


static void * rrepos_getrequest(void * hdl)
{
  GATHERCOMM   *comm;
  char          buffer[GATHERVALBUFLEN];
  size_t        bufferlen;
  char         *pluginname, *metricname;
  MetricValue  *mv;

  if (pthread_detach(pthread_self()) != 0) {
    perror("detaching thread");
  }

  //  fprintf(stderr,"--- start thread on socket %i\n",(int)hdl);
  while (1) {
    memset(buffer, 0, sizeof(buffer));
    bufferlen=sizeof(buffer);
    pthread_mutex_lock(&connect_mutex);
    if (rcs_getrequest((int)hdl,buffer,&bufferlen) == -1) {
      //      fprintf(stderr,"--- time out on socket %i\n",(int)hdl);
      connects--;
      pthread_cond_signal(&connect_cond);
      pthread_mutex_unlock(&connect_mutex);
      break;
    }
    //    fprintf(stderr,"---- received on socket %i: %s\n",(int)hdl,buffer);
    pthread_mutex_unlock(&connect_mutex);

    /* write data to repository */
    comm=(GATHERCOMM*)buffer;    
    comm->gc_cmd     = ntohs(comm->gc_cmd);
    comm->gc_datalen = ntohl(comm->gc_datalen);
    comm->gc_result  = ntohs(comm->gc_result);
        
    /* perform sanity check */
    if (bufferlen != sizeof(GATHERCOMM) + comm->gc_datalen) {
      fprintf(stderr,"--- invalid length received: expected %d got %d\n",
	      sizeof(GATHERCOMM)+comm->gc_datalen,bufferlen);
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
    //    fprintf(stderr,"socket %i : %s %s\n",(int)hdl,mv->mvSystemId,metricname);
    if ((comm->gc_result=reposvalue_put(pluginname,metricname,mv)) != 0) {
      fprintf(stderr,"write %s to repository failed\n",metricname);
    }
  }
  //  fprintf(stderr,"--- exit thread on socket %i\n",(int)hdl);
  return NULL;
}

static void * repos_remote()
{
  pthread_t thread;
  int       hdl = -1;

  while (1) {
    if (hdl == -1) {
      if (rcs_accept(&hdl) == -1) { 
	perror("_reposd_remote() rcs_accept"); 
	return 0; 
      }
    }
    pthread_mutex_lock(&connect_mutex);
    connects++;
    pthread_mutex_unlock(&connect_mutex);
    if (pthread_create(&thread,NULL,rrepos_getrequest,(void*)hdl) != 0) {
      perror("_reposd_remote create thread");
      return 0;
    }   
    pthread_mutex_lock(&connect_mutex);
    if(connects==rmaxconn) {
      /* wait for at least one finished thread */
      pthread_cond_wait(&connect_cond,&connect_mutex);
    }
    pthread_mutex_unlock(&connect_mutex);
    hdl = -1;
  }
  rcs_term();
  return 0;
}


/* ---------------------------------------------------------------------------*/

int main(int argc, char * argv[])
{
  int           quit=0;
  MC_REQHDR     hdr;
  GATHERCOMM   *comm;
  COMMHEAP     *ch;
  char          buffer[GATHERVALBUFLEN];
  size_t        bufferlen=sizeof(buffer);
  ValueRequest *vr;
  ValueItem    *vi;
  void         *vp, *vpmax;
  int           i,j;
  size_t        valreslen;
  RepositoryPluginDefinition *rdef;
  char         *pluginname, *metricname;
  MetricValue  *mv;
  pthread_t     rcomm;

  if (argc == 1) {
    /* daemonize if no arguments are given */
    if (daemon(0,0)) {
      perror("reposd");
      exit(-1);
    }
  }
  
  /* start remote reposd in separate thread */
  RINITCHECK();
  if (pthread_create(&rcomm,NULL,repos_remote,NULL) != 0) {
    perror("Could not create thread for remote reposd");
  }

  if (mcs_init(REPOS_COMMID)) {
    fprintf(stderr,"Could not open reposd socket.\n");
    return -1;
  }
  
  memset(buffer, 0, sizeof(buffer));
 
  while (!quit && mcs_accept(&hdr)==0) {
    while (!quit && mcs_getrequest(&hdr, buffer, &bufferlen)==0) {
      if (hdr.mc_type != GATHERMC_REQ) {
	/* ignore unknown message types */
	fprintf(stderr,"--- invalid request type  received %c\n",hdr.mc_type);
	continue;
      }
      comm=(GATHERCOMM*)buffer;
      /* perform sanity check */
      if (bufferlen != sizeof(GATHERCOMM) + comm->gc_datalen) {
	fprintf(stderr,"--- invalid length received: expected %d got %d\n",
		sizeof(GATHERCOMM)+comm->gc_datalen,bufferlen);
	continue;
      }
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
	comm->gc_result=reposplugin_add(buffer+sizeof(GATHERCOMM));
	comm->gc_datalen=0;
	break;
      case GCMD_REMPLUGIN:
	comm->gc_result=reposplugin_remove(buffer+sizeof(GATHERCOMM));
	comm->gc_datalen=0;
	break;
      case GCMD_LISTPLUGIN:
	ch=ch_init();
	comm->gc_result=reposplugin_list(buffer+sizeof(GATHERCOMM),
					 &rdef,
					 ch);
	if (comm->gc_result > 0) {
	  if (CHECKBUFFER(comm,buffer,strlen(buffer+sizeof(GATHERCOMM))+ 1 +
			  comm->gc_result*sizeof(RepositoryPluginDefinition))) {
	    comm->gc_datalen=strlen(buffer+sizeof(GATHERCOMM))+ 1 +
	      comm->gc_result*sizeof(RepositoryPluginDefinition);
	    memcpy(buffer+sizeof(GATHERCOMM)+strlen(buffer+sizeof(GATHERCOMM))+1,
		   rdef,
		   comm->gc_result*sizeof(RepositoryPluginDefinition));
	    for (i=0; i < comm->gc_result; i++) {
	      if (!CHECKBUFFER(comm,buffer,strlen(rdef[i].rdName) + 1)) {
		comm->gc_result=-1;
		break;
	      }
	      memcpy(buffer+sizeof(GATHERCOMM)+comm->gc_datalen,
		     rdef[i].rdName,
		     strlen(rdef[i].rdName) + 1);
	      comm->gc_datalen += strlen(rdef[i].rdName) + 1;
	      /* add pointer block for resources */
	      if (rdef[i].rdResource)
		for (j=0;rdef[i].rdResource[j];j++) {
		  if (!CHECKBUFFER(comm,buffer,strlen(rdef[i].rdResource[j]) + 1)) {
		    comm->gc_result=-1;
		    break;
		  }
		  memcpy(buffer+sizeof(GATHERCOMM)+comm->gc_datalen,
			 rdef[i].rdResource[j],
			 strlen(rdef[i].rdResource[j]) + 1);
		  comm->gc_datalen += strlen(rdef[i].rdResource[j]) + 1;	    
		}
	      else if CHECKBUFFER(comm,buffer,1) {
		memset(buffer+sizeof(GATHERCOMM)+comm->gc_datalen,
		       0,
		       1);
		comm->gc_datalen += 1;
	      } else {
		comm->gc_result=-1;
	      }
	    }  
	  } else {
	    comm->gc_result=-1;
	  }
	} else {
	  comm->gc_datalen=strlen(buffer+sizeof(GATHERCOMM))+ 1;
	}
	ch_release(ch);
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
	 * 2: ResourceName
	 * 3: ValueItems
	 */
	vr=(ValueRequest *)(buffer+sizeof(GATHERCOMM));
	vp=vr;
	vpmax=(void*)buffer+sizeof(buffer);
	if (vr->vsResource) {
	  /* adjust pointer to resource name */
	  vr->vsResource = (void*)vr + sizeof(ValueRequest);
	  valreslen= strlen(vr->vsResource) + 1;
	} else {
	  valreslen = 0;
	}
	vi=(void*)vr+sizeof(ValueRequest)+valreslen;
	vr->vsValues=NULL;
	ch=ch_init();
	comm->gc_result=reposvalue_get(vr,ch);
	/* copy value data into transfer buffer and compute total length */
	if (comm->gc_result != -1) {
	  if ((void*)vi+sizeof(ValueItem)*vr->vsNumValues < vpmax) {
	    memcpy(vi,vr->vsValues,sizeof(ValueItem)*vr->vsNumValues);
	    vp = (void*)vi + sizeof(ValueItem) * vr->vsNumValues;
	    for (i=0;i<vr->vsNumValues;i++) {
#ifdef DEBUG
	      fprintf(stderr,"returning value for mid=%d, resource %s: %d\n",
		      vr->vsId,
		      vr->vsValues[i].viResource,
		      *(int*)vr->vsValues[i].viValue);
#endif
	      if (vp + vr->vsValues[i].viValueLen < vpmax) {
		memcpy(vp,vr->vsValues[i].viValue,vr->vsValues[i].viValueLen);
		vi[i].viValue = vp;
		vp += vi[i].viValueLen;
		if (vr->vsValues[i].viResource) {
		  if (vp + strlen(vr->vsValues[i].viResource) + 1 < vpmax) {
		    strcpy(vp,vr->vsValues[i].viResource);
		    vi[i].viResource = vp;
		    vp += strlen(vp)+1;
		  }
		} else {
		  comm->gc_result=-1;
		  break;
		}
		if (vr->vsValues[i].viSystemId) {
		  if (vp + strlen(vr->vsValues[i].viSystemId) + 1 < vpmax) {
		    strcpy(vp,vr->vsValues[i].viSystemId);
		    vi[i].viSystemId = vp;
		    vp += strlen(vp)+1;
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
	  comm->gc_datalen=vp - (void*)vr;
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
	fprintf(stderr,"Error: Available data size is exceeding buffer.\n");
      }
      mcs_sendresponse(&hdr,buffer,sizeof(GATHERCOMM)+comm->gc_datalen);
      bufferlen=sizeof(buffer);
    }
  }
  mcs_term();
  return 0;
}
