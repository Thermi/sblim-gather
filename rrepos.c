/*
 * $Id: rrepos.c,v 1.8 2004/10/07 06:22:00 mihajlov Exp $
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
 * Description: Remote Repository Access Library
 * Implementation of the Remote API use to control the Repository
 * 
 */

#include "rrepos.h"
#include "gatherc.h"
#include "mcclt.h"
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/wait.h>

static RepositoryToken _RemoteToken = {sizeof(RepositoryToken),0,0};
static int rreposhandle=-1;
static char _systemId[260] = {0};


#define INITCHECK() \
if (rreposhandle==-1) { \
  rreposhandle=mcc_init(REPOS_COMMID); \
}

#ifdef NAGNAG
typedef struct _IdMap {
  int id_local;
  int id_remote;
} IdMap;

typedef struct _PluginMap {
  char      *pm_name;
  IdMap     *pm_map;
  PluginMap *pm_next;
} PluginMap;

PluginMap *pluginmap = NULL;

static PluginMap *pm_locate(const char *);
static PluginMap *pm_insert(const char *);
static int pm_delete(const char *);
static int pm_map(PluginMap *pm, int local, int remote);
#endif

int rrepos_sessioncheck()
{
  MC_REQHDR   hdr;
  char        xbuf[GATHERBUFLEN];
  GATHERCOMM *comm=(GATHERCOMM*)xbuf;
  size_t      commlen=sizeof(xbuf);
  RepositoryToken *rt=(RepositoryToken*)(xbuf+sizeof(GATHERCOMM));

  INITCHECK();
  hdr.mc_type=GATHERMC_REQ;
  comm->gc_cmd=GCMD_GETTOKEN;
  comm->gc_datalen=0;
  if (mcc_request(rreposhandle,&hdr,comm,sizeof(GATHERCOMM))==0 &&
      mcc_response(&hdr,comm,&commlen)==0 &&
      commlen == sizeof(RepositoryToken) + sizeof(GATHERCOMM)) {
    if (_RemoteToken.rt_size != ntohl(rt->rt_size)) {
      return -1; /* size mismatch */
    } else if (_RemoteToken.rt1 != rt->rt1 ||
	       _RemoteToken.rt2 != rt->rt2) {
      _RemoteToken.rt1 = rt->rt1;
      _RemoteToken.rt2 = rt->rt2;
      return 1;
    }
    return comm->gc_result;
  } else {
    return -1;
  }
  return 0;
}

int rrepos_register(const PluginDefinition *pdef)
{
  int rc = -1;
#ifdef NAGNAG
  if (pdef) {
    RepositoryPluginDefinition *rdef;
    PluginMap *pm;
    COMMHEAP ch=ch_init();
    if (rreposplugin_list(pdef->pdname,&rdef,ch) == 0) {
      pm_delete(pdef->pdname);
      pm=pm_insert(pdef->pdname);
      while (pdef->
      rc = 0;
    }    
    ch_term(ch);
  }
#endif
  return rc;
}

int rrepos_put(const char *reposplugin, const char *metric, MetricValue *mv)
{
  MC_REQHDR         hdr;
  char              xbuf[GATHERBUFLEN];
  GATHERCOMM       *comm=(GATHERCOMM*)xbuf;
  size_t            commlen=sizeof(xbuf);
  size_t            dataoffs=sizeof(GATHERCOMM);

  if (mv) {
    /* determine system id */
    if (strlen(_systemId)==0) {
      gethostname(_systemId,sizeof(_systemId));
    }
    INITCHECK();
    hdr.mc_type=GATHERMC_REQ;
    comm->gc_cmd=GCMD_SETVALUE;
    comm->gc_datalen=strlen(reposplugin) + 1 +
      strlen(metric) + 1 +
      sizeof(MetricValue) + 
      (mv->mvResource?strlen(mv->mvResource) + 1:0) +
      mv->mvDataLength +
      strlen(_systemId) + 1;
    /* prepare data */    
    strcpy(xbuf+dataoffs,reposplugin);
    dataoffs+=strlen(reposplugin)+1;
    strcpy(xbuf+dataoffs,metric);
    dataoffs+=strlen(metric)+1;
    memcpy(xbuf+dataoffs,mv,sizeof(MetricValue));
    dataoffs+=sizeof(MetricValue);
    if (mv->mvResource) {
      strcpy(xbuf+dataoffs,mv->mvResource);
      dataoffs+=strlen(mv->mvResource)+1;
    }
    memcpy(xbuf+dataoffs,mv->mvData,mv->mvDataLength);
    dataoffs+=mv->mvDataLength;
    strcpy(xbuf+dataoffs,_systemId);
    if (mcc_request(rreposhandle,&hdr,comm,
		    sizeof(GATHERCOMM)+comm->gc_datalen)==0 &&
	mcc_response(&hdr,comm,&commlen)==0 &&
	commlen == sizeof(GATHERCOMM)) {
      return comm->gc_result;
    } 
  }    
  return -1;
}

int rrepos_get(ValueRequest *vr, COMMHEAP ch)
{
  MC_REQHDR     hdr;
  char          xbuf[GATHERVALBUFLEN];
  GATHERCOMM   *comm=(GATHERCOMM*)xbuf;
  size_t        commlen=sizeof(xbuf);
  size_t        resourcelen;
  void         *vp;
  int           i;

  if (vr) {
    INITCHECK();
    resourcelen = vr->vsResource ? strlen(vr->vsResource) + 1 : 0;
    hdr.mc_type=GATHERMC_REQ;
    comm->gc_cmd=GCMD_GETVALUE;
    comm->gc_datalen=sizeof(ValueRequest) + resourcelen;
    /* copy parameters into xmit buffer and perform fixup for string */
    memcpy(xbuf+sizeof(GATHERCOMM),vr,sizeof(ValueRequest));
    if (resourcelen) {
      /* empty resource is allowed */
      strcpy(xbuf+sizeof(GATHERCOMM) + sizeof(ValueRequest),vr->vsResource);
    }
    if (mcc_request(rreposhandle,&hdr,comm,
		    sizeof(GATHERCOMM)+comm->gc_datalen)==0 &&
	mcc_response(&hdr,comm,&commlen)==0 &&
	commlen == (sizeof(GATHERCOMM) + comm->gc_datalen)) {
      if (comm->gc_result==0) {
	/* copy received ValueRequest into callers buffer 
	 * allocate array elements and adjust pointers */
	memcpy(vr,xbuf+sizeof(GATHERCOMM),sizeof(ValueRequest));
	vr->vsValues=ch_alloc(ch,vr->vsNumValues*sizeof(ValueItem));
	/* must consider resource name length */
	vp = xbuf + sizeof(GATHERCOMM) + sizeof(ValueRequest) + resourcelen;
	memcpy(vr->vsValues,vp,sizeof(ValueItem)*vr->vsNumValues);
	vp += vr->vsNumValues*sizeof(ValueItem);
	for (i=0; i<vr->vsNumValues; i++) {
	  vr->vsValues[i].viValue=ch_alloc(ch,vr->vsValues[i].viValueLen);
	  memcpy(vr->vsValues[i].viValue,vp,vr->vsValues[i].viValueLen);
	  vp += vr->vsValues[i].viValueLen;  
	  if (vr->vsValues[i].viResource) {
	    vr->vsValues[i].viResource = ch_alloc(ch,strlen(vp)+1);
	    strcpy(vr->vsValues[i].viResource,vp);
	    vp += strlen(vp) + 1;
	  }
	  if (vr->vsValues[i].viSystemId) {
	    vr->vsValues[i].viSystemId = ch_alloc(ch,strlen(vp)+1);
	    strcpy(vr->vsValues[i].viSystemId,vp);
	    vp += strlen(vp) + 1;
	  }
	}
	return comm->gc_result;
      }
    }
  }        
  return -1;  
}

int rrepos_init()
{
  MC_REQHDR   hdr;
  char        xbuf[GATHERBUFLEN];
  GATHERCOMM *comm=(GATHERCOMM*)xbuf;
  size_t      commlen=sizeof(xbuf);

  INITCHECK();
  hdr.mc_type=GATHERMC_REQ;
  comm->gc_cmd=GCMD_INIT;
  comm->gc_datalen=0;
  if (mcc_request(rreposhandle,&hdr,comm,sizeof(GATHERCOMM))==0 &&
      mcc_response(&hdr,comm,&commlen)==0) {
    return comm->gc_result;
  } else {
    return -1;
  }
}

int rrepos_terminate()
{
  MC_REQHDR   hdr;
  char        xbuf[GATHERBUFLEN];
  GATHERCOMM *comm=(GATHERCOMM*)xbuf;
  size_t      commlen=sizeof(xbuf);

  INITCHECK();
  hdr.mc_type=GATHERMC_REQ;
  comm->gc_cmd=GCMD_TERM;
  comm->gc_datalen=0;
  if (mcc_request(rreposhandle,&hdr,comm,sizeof(GATHERCOMM))==0 &&
      mcc_response(&hdr,comm,&commlen)==0) {
    return comm->gc_result;
  } else {
    return -1;
  }
}

int rrepos_status(RepositoryStatus *rs)
{
  MC_REQHDR         hdr;
  char              xbuf[GATHERBUFLEN];
  GATHERCOMM       *comm=(GATHERCOMM*)xbuf;
  RepositoryStatus *rsp=(RepositoryStatus*)(xbuf+sizeof(GATHERCOMM));
  size_t            commlen=sizeof(xbuf);

  if (rs) {
    INITCHECK();
    hdr.mc_type=GATHERMC_REQ;
    comm->gc_cmd=GCMD_STATUS;
    comm->gc_datalen=0;
    if (mcc_request(rreposhandle,&hdr,comm,sizeof(GATHERCOMM))==0 &&
	mcc_response(&hdr,comm,&commlen)==0 &&
	commlen == sizeof(RepositoryStatus) + sizeof(GATHERCOMM)) {
      *rs = *rsp;
      return comm->gc_result;
    } 
  }    
  return -1;
}

int rreposplugin_add(const char *pluginname)
{
  MC_REQHDR     hdr;
  char          xbuf[GATHERBUFLEN];
  GATHERCOMM   *comm=(GATHERCOMM*)xbuf;
  size_t        commlen=sizeof(xbuf);
  
  if (pluginname && *pluginname) {
    INITCHECK();
    hdr.mc_type=GATHERMC_REQ;
    comm->gc_cmd=GCMD_ADDPLUGIN;
    comm->gc_datalen=strlen(pluginname)+1;
    memcpy(xbuf+sizeof(GATHERCOMM),pluginname,comm->gc_datalen);
    if (mcc_request(rreposhandle,&hdr,comm,
		    sizeof(GATHERCOMM)+comm->gc_datalen)==0 &&
	mcc_response(&hdr,comm,&commlen)==0) {
      return comm->gc_result;
    } 
  }
  return -1;
}

int rreposplugin_remove(const char *pluginname)
{
  MC_REQHDR     hdr;
  char          xbuf[GATHERBUFLEN];
  GATHERCOMM   *comm=(GATHERCOMM*)xbuf;
  size_t        commlen=sizeof(xbuf);

  if (pluginname && *pluginname) {
    INITCHECK();
    hdr.mc_type=GATHERMC_REQ;
    comm->gc_cmd=GCMD_REMPLUGIN;
    comm->gc_datalen=strlen(pluginname)+1;
    memcpy(xbuf+sizeof(GATHERCOMM),pluginname,comm->gc_datalen);
    if (mcc_request(rreposhandle,&hdr,comm,
		    sizeof(GATHERCOMM)+comm->gc_datalen)==0 &&
	mcc_response(&hdr,comm,&commlen)==0) {
      return comm->gc_result;
    }
  }        
  return -1;  
}

int rreposplugin_list(const char *pluginname, 
		      RepositoryPluginDefinition **rdef, 
		      COMMHEAP ch)
{
  MC_REQHDR     hdr;
  char          xbuf[GATHERVALBUFLEN];
  GATHERCOMM   *comm=(GATHERCOMM*)xbuf;
  size_t        commlen=sizeof(xbuf);
  int           i,j;
  char         *stringpool;

  if (pluginname && *pluginname && rdef) {
    INITCHECK();
    hdr.mc_type=GATHERMC_REQ;
    comm->gc_cmd=GCMD_LISTPLUGIN;
    comm->gc_datalen=strlen(pluginname)+1;
    memcpy(xbuf+sizeof(GATHERCOMM),pluginname,comm->gc_datalen);
    if (mcc_request(rreposhandle,&hdr,comm,
		    sizeof(GATHERCOMM)+comm->gc_datalen)==0 &&
	mcc_response(&hdr,comm,&commlen)==0 &&
	commlen == (sizeof(GATHERCOMM) + comm->gc_datalen)) {
      /* copy data into result buffer and adjust string pointers */
      if (comm->gc_result>0) {
	*rdef = 
	  ch_alloc(ch,comm->gc_result*sizeof(RepositoryPluginDefinition));
	memcpy(*rdef,xbuf+sizeof(GATHERCOMM)+strlen(pluginname)+1,
	       sizeof(RepositoryPluginDefinition)*comm->gc_result);
	stringpool=
	  ch_alloc(ch,
		   comm->gc_datalen - 
		   sizeof(GATHERCOMM)+strlen(pluginname)+1+
		   comm->gc_result*sizeof(RepositoryPluginDefinition));
	memcpy(stringpool,xbuf+sizeof(GATHERCOMM)+strlen(pluginname)+1+
	       comm->gc_result*sizeof(RepositoryPluginDefinition),
	       comm->gc_datalen - sizeof(GATHERCOMM)+strlen(pluginname)+1+
	       comm->gc_result*sizeof(RepositoryPluginDefinition));
	for (i=0;i<comm->gc_result;i++) {
	  /* extract plugin name and resource names from string pool */
	  (*rdef)[i].rdName = stringpool;
	  stringpool += strlen(stringpool) + 1;
	  (*rdef)[i].rdResource=ch_alloc(ch,sizeof(char*)*100);
	  for (j=0;strlen(stringpool)>0;j++) {
	    (*rdef)[i].rdResource[j]=stringpool;
	    stringpool += sizeof(char*) + strlen(stringpool) + 1;
	  }
	  (*rdef)[i].rdResource[j]=NULL;
	  stringpool += 1;
	}
      }
      return comm->gc_result;
    }
  }        
  return -1;  
  
}

int rrepos_load()
{
  int pid;
  if (system("ps -C reposd")) {
    /* No reposd around */
    /*signal(SIGCHLD,SIG_IGN);*/
    pid=fork();
    switch(pid) {
    case 0:
      execlp("reposd","reposd",NULL);
      exit(-1);
      break;
    case -1:
      return -1;
      break;
    default:
      waitpid(pid,NULL,0);
      sleep(1); /* todo: need to make sure daemon is initialized */
      break;
    }
  }
  return 0;
}

int rrepos_unload()
{
  MC_REQHDR   hdr;
  char        xbuf[GATHERBUFLEN];
  GATHERCOMM *comm=(GATHERCOMM*)xbuf;
  size_t      commlen=sizeof(xbuf);

  INITCHECK();
  hdr.mc_type=GATHERMC_REQ;
  comm->gc_cmd=GCMD_QUIT;
  comm->gc_datalen=0;
  if (mcc_request(rreposhandle,&hdr,comm,sizeof(GATHERCOMM))==0 &&
      mcc_response(&hdr,comm,&commlen)==0) {
    return comm->gc_result;
  } else {
    return -1;
  }
}
