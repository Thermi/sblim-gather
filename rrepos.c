/*
 * $Id: rrepos.c,v 1.1 2004/07/16 15:30:05 mihajlov Exp $
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

int rrepos_sessioncheck()
{
  return 0;
}

int rrepos_register(const PluginDefinition *rdef)
{
  return 0;
}

int rrepos_put(MetricValue *mv)
{
  return 0;
}

int rrepos_get(ValueRequest *vs, COMMHEAP ch)
{
  return 0;
}

int rrepos_init()
{
  MC_REQHDR   hdr;
  char        xbuf[GATHERBUFLEN];
  GATHERCOMM *comm=(GATHERCOMM*)xbuf;
  size_t      commlen=sizeof(xbuf);

  hdr.mc_type=GATHERMC_REQ;
  comm->gc_cmd=GCMD_INIT;
  comm->gc_datalen=0;
  if (mcc_init(REPOS_COMMID) == 0 && 
      mcc_request(&hdr,comm,sizeof(GATHERCOMM))==0 &&
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

  hdr.mc_type=GATHERMC_REQ;
  comm->gc_cmd=GCMD_TERM;
  comm->gc_datalen=0;
  if (mcc_request(&hdr,comm,sizeof(GATHERCOMM))==0 &&
      mcc_response(&hdr,comm,&commlen)==0 &&
      mcc_term() == 0 ) {
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
    hdr.mc_type=GATHERMC_REQ;
    comm->gc_cmd=GCMD_STATUS;
    comm->gc_datalen=0;
    if (mcc_request(&hdr,comm,sizeof(GATHERCOMM))==0 &&
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

int rreposplugin_remove(const char *pluginname)
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
    hdr.mc_type=GATHERMC_REQ;
    comm->gc_cmd=GCMD_LISTPLUGIN;
    comm->gc_datalen=strlen(pluginname)+1;
    memcpy(xbuf+sizeof(GATHERCOMM),pluginname,comm->gc_datalen);
    if (mcc_request(&hdr,comm,sizeof(GATHERCOMM)+comm->gc_datalen)==0 &&
	mcc_response(&hdr,comm,&commlen)==0 &&
	commlen == (sizeof(GATHERCOMM) + comm->gc_datalen)) {
      /* copy data into result buffer and adjust string pointers */
      if (comm->gc_result>0) {
	*rdef = ch_alloc(ch,comm->gc_result*sizeof(PluginDefinition));
	memcpy(*rdef,xbuf+sizeof(GATHERCOMM)+strlen(pluginname)+1,
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
