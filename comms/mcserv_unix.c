/*
 * $Id: mcserv_unix.c,v 1.2 2004/07/16 15:30:05 mihajlov Exp $
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
 * Description: Gatherer Server-Side Communication APIs
 * Implementation based on AF_UNIX sockets.
 *
 */

#include "mcserv.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/file.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

static int commhandle = -1;
static int fdlockfile = -1;
static char sockname[PATH_MAX+1] = {0};
static char lockname[PATH_MAX+1] = {0};

int mcs_init(const char *commid)
{
  struct sockaddr_un sa;
  if (commhandle==-1) {
    commhandle=socket(PF_UNIX,SOCK_STREAM,0);
    if (commhandle==-1) {
      perror("Could not open socket");
      return -1;
    }
    if (fdlockfile == -1) {
      if (snprintf(lockname,sizeof(lockname),MC_LOCKFILE,commid) > 
	  sizeof(lockname)) {
	perror("Could not complete lockfile name");
	return -1;
      }
      fdlockfile = open(lockname,O_CREAT|O_RDWR, 0664);
      if (fdlockfile==-1) {
	perror("Could not open lockfile");
	return -1;
      }
    }
    if (flock(fdlockfile,LOCK_EX|LOCK_NB)) {
      perror("lock already in use");
      return -1;
    } 
    if (snprintf(sockname,sizeof(sockname),MC_SOCKET,commid) > 
	sizeof(sockname)) {
      perror("Could not complete socket name");
      return -1;
    }
    unlink(sockname);
    sa.sun_family=AF_UNIX;
    strcpy(sa.sun_path,sockname);
    if (bind(commhandle,(struct sockaddr*)&sa,sizeof(sa))) {
      perror("Could not bind socket");
      return -1;
    }
    listen(commhandle,0);
  }
  return 0;
}

int mcs_term()
{
  if (commhandle != -1) {
    close(commhandle);
    commhandle = -1;
    if (lockname[0]) {
      unlink (lockname);
      lockname[0]=0;
    }
  }
  if (fdlockfile != -1) {
    flock(fdlockfile,LOCK_UN);
    close(fdlockfile);
    fdlockfile = -1;
    if (sockname[0]) {
      unlink (sockname);
      sockname[0]=0;
    }
  }
  return 0;
}

int mcs_getrequest(MC_REQHDR *hdr, void *reqdata, size_t *reqdatalen)
{
  int           srvhandle=-1;
  size_t        readlen=0;
  size_t        recvlen=0;
  int           maxlen=0;
  if (hdr && commhandle != -1 && reqdata && reqdatalen) {
    if (*reqdatalen>0) maxlen=*reqdatalen;
    srvhandle=accept(commhandle,NULL,0);
    if (srvhandle != -1) {
      do {
	/* get header */
	readlen=read(srvhandle,(void*)hdr+recvlen,sizeof(MC_REQHDR)-recvlen);
	if (readlen <= 0) break;
	recvlen += readlen;
      } while (recvlen != sizeof(MC_REQHDR));
      if (readlen > 0) {
	hdr->mc_handle = srvhandle;
	readlen=0;
	recvlen=0;
	do {
	  /* get length */
	  readlen=read(srvhandle,(void*)reqdatalen+recvlen,
		       sizeof(size_t)-recvlen);
	  if (readlen <= 0) break;
	  recvlen += readlen;
	} while (recvlen != sizeof(size_t));
	if (maxlen > 0 && *reqdatalen > maxlen) {
	  fprintf(stderr,
		  "getrequest buffer to small, needed %d available %d\n",
		  *reqdatalen,
		  maxlen);
 	} else if( readlen >0 ) {
	  readlen=0;
	  recvlen=0;
	  do {
	    /* get data */
	    readlen=read(srvhandle,reqdata+recvlen,*reqdatalen-recvlen);
	    if (readlen <= 0) break;
	    recvlen += readlen;
	  } while (recvlen != *reqdatalen);
	  if (readlen > 0) 
	    return 0;
	}
      }
    }
  }
  perror("mcserv request");
  return -1;
}

int mcs_sendresponse(MC_REQHDR *hdr, void *respdata, size_t respdatalen)
{
  int    sentlen;
  if (hdr && hdr->mc_handle != -1 && respdata) {
    sentlen = write(hdr->mc_handle,hdr,sizeof(MC_REQHDR)) +
      write(hdr->mc_handle,&respdatalen,sizeof(size_t)) +
      write(hdr->mc_handle,respdata,respdatalen);
    close(hdr->mc_handle);
    if (sentlen == (respdatalen+sizeof(size_t)+sizeof(MC_REQHDR)))
      return 0;
    fprintf(stderr,"sendresponse error, wanted %d got %d\n",
	    respdatalen+sizeof(size_t)+sizeof(MC_REQHDR),
	    sentlen);
  }
  perror("mcserv response");
  return -1;
}
