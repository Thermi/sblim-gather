/*
 * $Id: gatherd.c,v 1.4 2004/10/08 11:06:41 mihajlov Exp $
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
 * Description: Gatherer Daemon
 *
 * The Performance Data Gatherer Daemon is processing requests from
 * a client and returning request results and status information.
 * 
 */

#include "gather.h"
#include "gatherc.h"
#include "commheap.h"
#include <mcserv.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define CHECKBUFFER(comm,buffer,sz) ((comm)->gc_datalen+sizeof(GATHERCOMM)+(sz)<=sizeof(buffer))

int main(int argc, char * argv[])
{
  int           quit=0;
  MC_REQHDR     hdr;
  GATHERCOMM   *comm;
  COMMHEAP     *ch;
  char          buffer[GATHERVALBUFLEN];
  size_t        bufferlen=sizeof(buffer);
  int           i, j;
  PluginDefinition *pdef;
  
  if (argc == 1) {
    /* daemonize if no arguments are given */
    if (daemon(0,0)) {
      perror("gatherd");
      exit(-1);
    }
  }
  
  if (mcs_init(GATHER_COMMID)) {
    return -1;
  }

  memset(buffer, 0, sizeof(buffer));

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
      gather_status((GatherStatus*)(buffer+sizeof(GATHERCOMM)));
      comm->gc_result=0;
      comm->gc_datalen=sizeof(GatherStatus);
      break;
    case GCMD_INIT:
      comm->gc_result=gather_init();
      comm->gc_datalen=0;
      break;
    case GCMD_TERM:
      comm->gc_result=gather_terminate();
      comm->gc_datalen=0;
      break;
    case GCMD_START:
      comm->gc_result=gather_start();
      comm->gc_datalen=0;
      break;
    case GCMD_STOP:
      comm->gc_result=gather_stop();
      comm->gc_datalen=0;
      break;
    case GCMD_ADDPLUGIN:
      comm->gc_result=metricplugin_add(buffer+sizeof(GATHERCOMM));
      comm->gc_datalen=0;
      break;
    case GCMD_REMPLUGIN:
      comm->gc_result=metricplugin_remove(buffer+sizeof(GATHERCOMM));
      comm->gc_datalen=0;
      break;
    case GCMD_LISTPLUGIN:
      ch=ch_init();
      comm->gc_result=metricplugin_list(buffer+sizeof(GATHERCOMM),
					&pdef,
					ch);
      if (comm->gc_result > 0) {
	if (CHECKBUFFER(comm,buffer,strlen(buffer+sizeof(GATHERCOMM))+ 1 +
			comm->gc_result*sizeof(PluginDefinition))) {
	  comm->gc_datalen=strlen(buffer+sizeof(GATHERCOMM))+ 1 +
	    comm->gc_result*sizeof(PluginDefinition);
	  memcpy(buffer+sizeof(GATHERCOMM)+strlen(buffer+sizeof(GATHERCOMM))+1,
		 pdef,
		 comm->gc_result*sizeof(PluginDefinition));
	  for (i=0; i < comm->gc_result; i++) {
	    if (!CHECKBUFFER(comm,buffer,strlen(pdef[i].pdName) + 1)) {
	      comm->gc_result=-1;
	      break;
	    }
	    memcpy(buffer+sizeof(GATHERCOMM)+comm->gc_datalen,
		   pdef[i].pdName,
		   strlen(pdef[i].pdName) + 1);
	    comm->gc_datalen += strlen(pdef[i].pdName) + 1;
	    /* add pointer block for resources */
	    if (pdef[i].pdResource)
	      for (j=0;pdef[i].pdResource[j];j++) {
		if (!CHECKBUFFER(comm,buffer,strlen(pdef[i].pdResource[j]) + 1)) {
		  comm->gc_result=-1;
		  break;
		}
		memcpy(buffer+sizeof(GATHERCOMM)+comm->gc_datalen,
		       pdef[i].pdResource[j],
		       strlen(pdef[i].pdResource[j]) + 1);
		comm->gc_datalen += strlen(pdef[i].pdResource[j]) + 1;	    
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
  mcs_term();
  return 0;
}
