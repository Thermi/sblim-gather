/*
 * $Id: rgather.c,v 1.2 2004/05/14 12:11:09 mihajlov Exp $
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
 * Description: Remote Gatherer Access Library
 * Implementation of the Remote API use to control the Gatherer. 
 * 
 */

#include "rgather.h"
#include "gatherc.h"
#include "mcclt.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

int rgather_load()
{
  int pid;
  if (system("ps -C gatherd")) {
    /* No gatherd around */
    /*signal(SIGCHLD,SIG_IGN);*/
    pid=fork();
    switch(pid) {
    case 0:
      execlp("gatherd","gatherd",NULL);
      exit(-1);
      break;
    case -1:
      return -1;
      break;
    }
  }
  return 0;
}

int rgather_unload()
{
  MC_REQHDR   hdr;
  char        xbuf[GATHERBUFLEN];
  GATHERCOMM *comm=(GATHERCOMM*)xbuf;
  size_t      commlen=sizeof(xbuf);

  hdr.mc_type=GATHERMC_REQ;
  comm->gc_cmd=GCMD_QUIT;
  comm->gc_datalen=0;
  if (mcc_request(&hdr,comm,sizeof(GATHERCOMM))==0 &&
      mcc_response(&hdr,comm,&commlen)==0) {
    return comm->gc_result;
  } else {
    return -1;
  }
}

int rgather_init()
{
  MC_REQHDR   hdr;
  char        xbuf[GATHERBUFLEN];
  GATHERCOMM *comm=(GATHERCOMM*)xbuf;
  size_t      commlen=sizeof(xbuf);

  hdr.mc_type=GATHERMC_REQ;
  comm->gc_cmd=GCMD_INIT;
  comm->gc_datalen=0;
  if (mcc_request(&hdr,comm,sizeof(GATHERCOMM))==0 &&
      mcc_response(&hdr,comm,&commlen)==0) {
    return comm->gc_result;
  } else {
    return -1;
  }
}

int rgather_terminate()
{
  MC_REQHDR   hdr;
  char        xbuf[GATHERBUFLEN];
  GATHERCOMM *comm=(GATHERCOMM*)xbuf;
  size_t      commlen=sizeof(xbuf);

  hdr.mc_type=GATHERMC_REQ;
  comm->gc_cmd=GCMD_TERM;
  comm->gc_datalen=0;
  if (mcc_request(&hdr,comm,sizeof(GATHERCOMM))==0 &&
      mcc_response(&hdr,comm,&commlen)==0) {
    return comm->gc_result;
  } else {
    return -1;
  }
}

int rgather_start()
{
  MC_REQHDR   hdr;
  char        xbuf[GATHERBUFLEN];
  GATHERCOMM *comm=(GATHERCOMM*)xbuf;
  size_t      commlen=sizeof(xbuf);

  hdr.mc_type=GATHERMC_REQ;
  comm->gc_cmd=GCMD_START;
  comm->gc_datalen=0;
  if (mcc_request(&hdr,comm,sizeof(GATHERCOMM))==0 &&
      mcc_response(&hdr,comm,&commlen)==0) {
    return comm->gc_result;
  } else {
    return -1;
  }
}

int rgather_stop()
{
  MC_REQHDR   hdr;
  char        xbuf[GATHERBUFLEN];
  GATHERCOMM *comm=(GATHERCOMM*)xbuf;
  size_t      commlen=sizeof(xbuf);

  hdr.mc_type=GATHERMC_REQ;
  comm->gc_cmd=GCMD_STOP;
  comm->gc_datalen=0;
  if (mcc_request(&hdr,comm,sizeof(GATHERCOMM))==0 &&
      mcc_response(&hdr,comm,&commlen)==0) {
    return comm->gc_result;
  } else {
    return -1;
  }
}

int rgather_status(GatherStatus *gs)
{
  MC_REQHDR     hdr;
  char          xbuf[GATHERBUFLEN];
  GATHERCOMM   *comm=(GATHERCOMM*)xbuf;
  GatherStatus *gsp=(GatherStatus*)(xbuf+sizeof(GATHERCOMM));
  size_t        commlen=sizeof(xbuf);

  if (gs) {
    hdr.mc_type=GATHERMC_REQ;
    comm->gc_cmd=GCMD_STATUS;
    comm->gc_datalen=0;
    if (mcc_request(&hdr,comm,sizeof(GATHERCOMM))==0 &&
	mcc_response(&hdr,comm,&commlen)==0 &&
	commlen == sizeof(GatherStatus) + sizeof(GATHERCOMM)) {
      *gs = *gsp;
      return comm->gc_result;
    } 
  }    
  return -1;
}

int rmetricplugin_add(const char *pluginname)
{
  MC_REQHDR     hdr;
  char          xbuf[GATHERBUFLEN];
  GATHERCOMM   *comm=(GATHERCOMM*)xbuf;
  size_t        commlen=sizeof(xbuf);
  
  if (pluginname && *pluginname) {
    hdr.mc_type=GATHERMC_REQ;
    comm->gc_cmd=GCMD_ADDPLUGIN;
    comm->gc_datalen=strlen(pluginname)+1;
    memcpy(xbuf+sizeof(GATHERCOMM),pluginname,comm->gc_datalen);
    if (mcc_request(&hdr,comm,sizeof(GATHERCOMM)+comm->gc_datalen)==0 &&
	mcc_response(&hdr,comm,&commlen)==0) {
      return comm->gc_result;
    } 
  }
  return -1;
}

int rmetricplugin_remove(const char *pluginname)
{
  MC_REQHDR     hdr;
  char          xbuf[GATHERBUFLEN];
  GATHERCOMM   *comm=(GATHERCOMM*)xbuf;
  size_t        commlen=sizeof(xbuf);

  if (pluginname && *pluginname) {
    hdr.mc_type=GATHERMC_REQ;
    comm->gc_cmd=GCMD_REMPLUGIN;
    comm->gc_datalen=strlen(pluginname)+1;
    memcpy(xbuf+sizeof(GATHERCOMM),pluginname,comm->gc_datalen);
    if (mcc_request(&hdr,comm,sizeof(GATHERCOMM)+comm->gc_datalen)==0 &&
	mcc_response(&hdr,comm,&commlen)==0) {
      return comm->gc_result;
    }
  }        
  return -1;  
}

int rgather_get(ValueRequest *vr, COMMHEAP ch)
{
  MC_REQHDR     hdr;
  char          xbuf[GATHERVALBUFLEN];
  GATHERCOMM   *comm=(GATHERCOMM*)xbuf;
  size_t        commlen=sizeof(xbuf);
  size_t        resourcelen;
  void         *vp;
  int           i;

  if (vr) {
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
    if (mcc_request(&hdr,comm,sizeof(GATHERCOMM)+comm->gc_datalen)==0 &&
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
	}
	return comm->gc_result;
      }
    }
  }        
  return -1;  
  
}

int rmetricplugin_list(const char *pluginname, PluginDefinition **pdef, 
		       COMMHEAP ch)
{
  MC_REQHDR     hdr;
  char          xbuf[GATHERVALBUFLEN];
  GATHERCOMM   *comm=(GATHERCOMM*)xbuf;
  size_t        commlen=sizeof(xbuf);
  int           i,j;
  char         *stringpool;

  if (pluginname && *pluginname && pdef) {
    hdr.mc_type=GATHERMC_REQ;
    comm->gc_cmd=GCMD_LISTPLUGIN;
    comm->gc_datalen=strlen(pluginname)+1;
    memcpy(xbuf+sizeof(GATHERCOMM),pluginname,comm->gc_datalen);
    if (mcc_request(&hdr,comm,sizeof(GATHERCOMM)+comm->gc_datalen)==0 &&
	mcc_response(&hdr,comm,&commlen)==0 &&
	commlen == (sizeof(GATHERCOMM) + comm->gc_datalen)) {
      /* copy data into result buffer and adjust string pointers */
      if (comm->gc_result>0) {
	*pdef = ch_alloc(ch,comm->gc_result*sizeof(PluginDefinition));
	memcpy(*pdef,xbuf+sizeof(GATHERCOMM)+strlen(pluginname)+1,
	       sizeof(PluginDefinition)*comm->gc_result);
	stringpool=ch_alloc(ch,
			    comm->gc_datalen - 
			    sizeof(GATHERCOMM)+strlen(pluginname)+1+
			    comm->gc_result*sizeof(PluginDefinition));
	memcpy(stringpool,xbuf+sizeof(GATHERCOMM)+strlen(pluginname)+1+
	       comm->gc_result*sizeof(PluginDefinition),
	       comm->gc_datalen - sizeof(GATHERCOMM)+strlen(pluginname)+1+
	       comm->gc_result*sizeof(PluginDefinition));
	for (i=0;i<comm->gc_result;i++) {
	  /* extract plugin name and resource names from string pool */
	  (*pdef)[i].pdName = stringpool;
	  stringpool += strlen(stringpool) + 1;
	  (*pdef)[i].pdResource=ch_alloc(ch,sizeof(char*)*100);
	  for (j=0;strlen(stringpool)>0;j++) {
	    (*pdef)[i].pdResource[j]=stringpool;
	    stringpool += sizeof(char*) + strlen(stringpool) + 1;
	  }
	  (*pdef)[i].pdResource[j]=NULL;
	  stringpool += 1;
	}
      }
      return comm->gc_result;
    }
  }        
  return -1;  
  
}
