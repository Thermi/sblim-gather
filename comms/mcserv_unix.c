/*
 * $Id: mcserv_unix.c,v 1.4 2004/10/20 09:08:16 mihajlov Exp $
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
#include <mlog.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/file.h>
#include <sys/poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

static int _sigpipe_h_installed = 0;
static int _sigpipe_h_received = 0;

static void _sigpipe_h(int signal)
{
  _sigpipe_h_received++; 
}

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
      m_log(M_ERROR,M_QUIET,
	    "mcs_init: could not create socket, error string %s\n",
	    strerror(errno));
      return -1;
    }
    if (fdlockfile == -1) {
      if (snprintf(lockname,sizeof(lockname),MC_LOCKFILE,commid) > 
	  sizeof(lockname)) {
	m_log(M_ERROR,M_QUIET,
	      "mcs_init: could not cimplete lockfile name %s\n"
	      MC_LOCKFILE);
	return -1;
      }
      fdlockfile = open(lockname,O_CREAT|O_RDWR, 0664);
      if (fdlockfile==-1) {
	m_log(M_ERROR,M_QUIET,
	      "mcs_init: could not open lockfile %s, error string %s\n",
	      lockname,
	      strerror(errno));
	return -1;
      }
    }
    if (flock(fdlockfile,LOCK_EX|LOCK_NB)) {
 	m_log(M_ERROR,M_QUIET,
	      "mcs_init: lockfile %s already in use, error string %s\n",
	      lockname,
	      strerror(errno));
      return -1;
    } 
    if (snprintf(sockname,sizeof(sockname),MC_SOCKET,commid) > 
	sizeof(sockname)) {
	m_log(M_ERROR,M_QUIET,
	      "mcs_init: could not cimplete socket name %s\n"
	      MC_SOCKET);
	return -1;
    }
    unlink(sockname);
    sa.sun_family=AF_UNIX;
    strcpy(sa.sun_path,sockname);
    if (bind(commhandle,(struct sockaddr*)&sa,sizeof(sa))) {
      m_log(M_ERROR,M_QUIET,
	    "mcs_init: could not bind socket %s, error string %s\n",
	    sockname,
	    strerror(errno));
      return -1;
    }
    if (!_sigpipe_h_installed) {
      signal(SIGPIPE,_sigpipe_h);
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

int mcs_accept(MC_REQHDR *hdr)
{
  if (hdr) {
    hdr->mc_handle=accept(commhandle,NULL,0);
    if (hdr->mc_handle == -1) {
      m_log(M_ERROR,M_QUIET,
	    "mcs_accept: failed to accept server socket, error string %s\n",
	    sockname,
	    strerror(errno));
      return -1;
    } 
    return 0;
  }
  
  return -1;
}

int mcs_getrequest(MC_REQHDR *hdr, void *reqdata, size_t *reqdatalen)
{
  struct iovec iov[2] = {
    {hdr,sizeof(MC_REQHDR)},
    {reqdatalen,sizeof(size_t)}
  };
  struct pollfd pf;
  int           srvhandle=-1;
  int           readlen=0;
  size_t        recvlen=0;
  int           maxlen=0;

  if (hdr && commhandle != -1 && reqdata && reqdatalen) {
    srvhandle = hdr->mc_handle;
    if (*reqdatalen>0) maxlen=*reqdatalen;
    if (srvhandle != -1) {
      /* use poll to implement a timeout disconnection */
      pf.fd=srvhandle;
      pf.events=POLLIN;
      if (poll(&pf,1,1000) == 1 && 
	  !(pf.revents & (POLLERR|POLLNVAL|POLLHUP))) {
	do {
	  /* get header & size */
	  readlen=readv(srvhandle,iov,2);
	  if (readlen <= 0) {
	    m_log(M_ERROR,M_QUIET,
		  "mcs_getrequest: failed to read header, error string %s\n",
		  sockname,
		  strerror(errno));
	    break;
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
	if (maxlen > 0 && *reqdatalen > maxlen) {
	    m_log(M_ERROR,M_QUIET,
		  "mcs_request: buffer too small, needed %d available %d\n",
		  *reqdatalen,
		  maxlen);
	} else { 
	  readlen=0;
	  recvlen=0;
	  hdr->mc_handle=srvhandle;
	  do {
	    /* get data */
	    readlen=read(srvhandle,reqdata+recvlen,*reqdatalen-recvlen);
	    if (readlen <= 0) break;
	    recvlen += readlen;
	  } while (recvlen != *reqdatalen);
	  if (readlen > 0) { 
	    return 0;
	  } else {
	    m_log(M_ERROR,M_QUIET,
		  "mcs_request: data read error, error string %s\n",
		  strerror(errno));
	  }
	}
      }
    }
  }
  if (srvhandle != -1) {
    close(srvhandle);
  }
  if (hdr) {
    hdr->mc_handle=-1;
  }
  return -1;
}

int mcs_sendresponse(MC_REQHDR *hdr, void *respdata, size_t respdatalen)
{
  int    sentlen;
  struct iovec iov[3] = {
    {hdr,sizeof(MC_REQHDR)},
    {&respdatalen,sizeof(size_t)},
    {respdata,respdatalen}
  };
  if (hdr && hdr->mc_handle != -1 && respdata) {
    sentlen = writev(hdr->mc_handle,iov,3);
    if (sentlen == (respdatalen+sizeof(size_t)+sizeof(MC_REQHDR)))
      return 0;
    m_log(M_ERROR,M_QUIET,
	  "mcs_sendresponse: send error, wanted %d got %d, error string %s\n",
	  respdatalen+sizeof(size_t)+sizeof(MC_REQHDR),
	  sentlen, strerror(errno));
  }
  return -1;
}
