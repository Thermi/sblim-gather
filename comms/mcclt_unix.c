/*
 * $Id: mcclt_unix.c,v 1.2 2004/07/16 15:30:05 mihajlov Exp $
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
 * Description:  Gatherer Client-Side Communication APIs
 * AF_UNIX version.
 *
 */

#include "mcclt.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


static char sockname[PATH_MAX+1] = {0};


int mcc_init(const char *commid)
{
  if (sockname[0]==0) {
    if (snprintf(sockname,sizeof(sockname),MC_SOCKET,commid) > 
	sizeof(sockname)) {
      perror("Could not complete socket name");
      return -1;
    }
    return 0;
  }
  return -1;
}

int mcc_term()
{
  if (sockname[0]) {
    sockname[0]=0;
    return 0;
  }
  return -1;
}

int mcc_request(MC_REQHDR *hdr, void *reqdata, size_t reqdatalen)
{
  int                cltsock =-1;
  size_t             sentlen;
  struct sockaddr_un sa;
  if (hdr && reqdata) {
    cltsock=socket(PF_UNIX,SOCK_STREAM,0);
    if (cltsock==-1) {
      perror("socket");
      return -1;
    }
    sa.sun_family = AF_UNIX;
    strcpy(sa.sun_path,sockname);
    if (connect(cltsock,(struct sockaddr*)&sa,sizeof(struct sockaddr_un))==0) {
      sentlen = write(cltsock,hdr,sizeof(MC_REQHDR)) +
	write(cltsock,&reqdatalen,sizeof(size_t)) +
	write(cltsock,reqdata,reqdatalen);
      if (sentlen == (reqdatalen+sizeof(size_t)+sizeof(MC_REQHDR))) {
	hdr->mc_handle=cltsock;
	return 0;
      }
      fprintf(stderr,"sendrequest error, wanted %d got %d\n",
	      reqdatalen+sizeof(size_t)+sizeof(MC_REQHDR),
	      sentlen);
    }
  }
  perror("mcserv send");
  return -1;
}

int mcc_response(MC_REQHDR *hdr, void *respdata, size_t *respdatalen)
{

  int    cltsock;
  size_t readlen=0;
  size_t recvlen=0;
  int    maxlen=0;
  if (hdr && hdr->mc_handle!=-1 && respdata && respdatalen) {
    if (*respdatalen>0) maxlen=*respdatalen;
    cltsock = hdr->mc_handle;
    do {
      /* get header */
      readlen=read(cltsock,(void*)hdr+recvlen,sizeof(MC_REQHDR)-recvlen);
      if (readlen <= 0) break;
      recvlen += readlen;
    } while (recvlen != sizeof(MC_REQHDR));
    if (readlen > 0) {
      hdr->mc_handle = cltsock;
      readlen=0;
      recvlen=0;
      do {
	/* get length */
	readlen=read(cltsock,(void*)respdatalen+recvlen,
		     sizeof(size_t)-recvlen);
	if (readlen <= 0) break;
	recvlen+=readlen;
      } while (recvlen != sizeof(size_t));
      if (maxlen > 0 && *respdatalen > maxlen) {
	fprintf(stderr,
		"getresponse buffer to small, needed %d available %d\n",
		*respdatalen,
		maxlen);
      }	else if (readlen > 0) {
	readlen=0;
	recvlen=0;
	do {
	  /* get data */
	  readlen=read(cltsock,respdata+recvlen,*respdatalen-recvlen);
	  if (readlen <= 0) break;
	  recvlen+=readlen;
	} while (recvlen != *respdatalen);
	if (readlen > 0) {      
	  if (close(cltsock)) perror("client socket close");
	  return 0;
	}
      }
    }
  }
  return -1;
}
