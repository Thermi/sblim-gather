/*
 * $Id: mcclt_unix.c,v 1.5 2004/10/20 08:31:06 mihajlov Exp $
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
#include <mlog.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>

#define MAXCONN 10

static int _sigpipe_h_installed = 0;
static int _sigpipe_h_received = 0;

static void _sigpipe_h(int signal)
{
  _sigpipe_h_received++; 
}

/* the mutexes only protect creation/deletion - not the I/O-s */
/* this must be done in the app layer */
static pthread_mutex_t sockname_mutex = PTHREAD_MUTEX_INITIALIZER;
static struct {
  char sn_name[PATH_MAX+1];
  int  sn_handle;
  /* runtime statistics */
  int  sn_connects;
  int  sn_requests;
} sockname[MAXCONN] = {{{0},0}};


int mcc_init(const char *commid)
{
  int i;
  pthread_mutex_lock(&sockname_mutex);
  for (i=0; i < MAXCONN;i++) {
    if (sockname[i].sn_name[0]==0) {
      if (snprintf(sockname[i].sn_name,sizeof(sockname),MC_SOCKET,commid) > 
	  sizeof(sockname[i].sn_name)) {
	m_log(M_ERROR,M_QUIET,
	      "mcc_init: could not complete socket name for %s\n",
	      commid);
	pthread_mutex_unlock(&sockname_mutex);
	return -1;
      }
      if (!_sigpipe_h_installed) {
	signal(SIGPIPE,_sigpipe_h);
      }
      pthread_mutex_unlock(&sockname_mutex);
      return i;
    }
  }
  pthread_mutex_unlock(&sockname_mutex);
  return -1;
}

int mcc_term(int commhandle)
{
  pthread_mutex_lock(&sockname_mutex);
  if (commhandle >= 0 && commhandle < MAXCONN && 
      sockname[commhandle].sn_name[0]) {
    sockname[commhandle].sn_name[0]=0;
    if (sockname[commhandle].sn_handle > 0) {
      close(sockname[commhandle].sn_handle);
      sockname[commhandle].sn_handle=-1;
    }
    pthread_mutex_unlock(&sockname_mutex);
    return 0;
  }
  pthread_mutex_unlock(&sockname_mutex);
  return -1;
}

static int _mcc_connect(int commhandle)
{    
  struct sockaddr_un sa;

  if (commhandle < MAXCONN && sockname[commhandle].sn_name[0]) {
    if (sockname[commhandle].sn_handle>0) {
      close(sockname[commhandle].sn_handle);
    }
    sockname[commhandle].sn_handle=socket(PF_UNIX,SOCK_STREAM,0);
    if (sockname[commhandle].sn_handle==-1) {
      m_log(M_ERROR,M_QUIET,
	    "_mcc_connect: could not create socket for %s, error string %s\n",
	    sockname[commhandle].sn_name,
	    strerror(errno));
      return -1;
    }
    sa.sun_family = AF_UNIX;
    strcpy(sa.sun_path,sockname[commhandle].sn_name);
    sockname[commhandle].sn_connects ++;
    return connect(sockname[commhandle].sn_handle,
		   (struct sockaddr*)&sa,
		   sizeof(struct sockaddr_un));

  }
  /* invalid commhandle */
  return -1;
}

int mcc_request(int commhandle, MC_REQHDR *hdr, 
		void *reqdata, size_t reqdatalen)
{
  int        sentlen;
  struct iovec iov [3] = { 
    {hdr, sizeof(MC_REQHDR)}, 
    {&reqdatalen, sizeof(size_t)}, 
    {reqdata, reqdatalen}, 
  };
  if (commhandle < 0 || commhandle >= MAXCONN || hdr == NULL || 
      reqdata == NULL) {
    return -1;
  }
  if (sockname[commhandle].sn_handle<=0) {
    if (_mcc_connect(commhandle)<0 ) {
      m_log(M_ERROR,M_QUIET,
	    "mcc_request: could not connect socket for %s, error string %s\n",
	    sockname[commhandle].sn_name,
	    strerror(errno));
      return -1;
    }
  }
  sentlen = writev(sockname[commhandle].sn_handle,iov,3);
  if (sentlen <= 0) {
    if (_mcc_connect(commhandle)<0 ) {
      m_log(M_ERROR,M_QUIET,
	    "mcc_init: could not reconnect socket for %s, error string %s\n",
	    sockname[commhandle].sn_name,
	    strerror(errno));
      return -1;
    } else {
      sentlen = writev(sockname[commhandle].sn_handle,iov,3);
    }
  }
  if (sentlen == (reqdatalen+sizeof(size_t)+sizeof(MC_REQHDR))) {
    hdr->mc_handle=sockname[commhandle].sn_handle;
    sockname[commhandle].sn_requests++;
    return 0;
  }
  m_log(M_ERROR,M_QUIET,
	"mcc_request: send error, wanted %d got %d, error string %s\n",
	reqdatalen+sizeof(size_t)+sizeof(MC_REQHDR),
	sentlen, strerror(errno));
  return -1;
}

int mcc_response(MC_REQHDR *hdr, void *respdata, size_t *respdatalen)
{
  struct iovec iov[2] = {
    {hdr,sizeof(MC_REQHDR)},
    {respdatalen,sizeof(size_t)}
  };
  int    cltsock;
  int    readlen=0;
  size_t  recvlen=0;
  int    maxlen=0;
  if (hdr==NULL || hdr->mc_handle ==-1 || respdata == NULL || 
      respdatalen == NULL) {
    return -1;
  }
  if (*respdatalen>0) maxlen=*respdatalen;
  cltsock = hdr->mc_handle;
  do {
    /* get header & size */
    readlen=readv(cltsock,iov,2);
    if (readlen <= 0) {
      return -1;
    }
    recvlen += readlen;
    if (iov[0].iov_len < readlen) {
      readlen -= iov[0].iov_len;
      iov[0].iov_len=0; 
      iov[1].iov_len-=readlen;
    } else {
      iov[0].iov_len-=readlen; 
    }
  } while (recvlen != (sizeof(MC_REQHDR)+sizeof(size_t)));
  if (maxlen > 0 && *respdatalen > maxlen) {
    m_log(M_ERROR, M_QUIET,
	  "mcc_repsonse: buffer to small, needed %d available %d\n",
	  *respdatalen,
	  maxlen);
    return -1;
  }  
  readlen=0;
  recvlen=0;
  do {
    /* get data */
    readlen=read(cltsock,respdata+recvlen,*respdatalen-recvlen);
    if (readlen <= 0) break;
    recvlen+=readlen;
  } while (recvlen != *respdatalen);
  if (readlen > 0) {      
    return 0;
  }
  return -1;
}
