/*
 * $Id: rcserv_ip.c,v 1.2 2004/10/15 10:40:38 heidineu Exp $
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
 * Author:       Viktor Mihajlovski <mihajlov@de.ibm.com>
 * Contributors: Heidi Neumann <heidineu@de.ibm.com>
 *
 * Description: Gatherer Server-Side Communication APIs
 *              TCP/IP Socket version
 *
 */

#include "rcserv.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/poll.h>
#include <sys/file.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>


/* ---------------------------------------------------------------------------*/

#define HOST_NAME_MAX  255
#define PORT 6363

static int srvhdl=-1;
static int connects;

static pthread_mutex_t connect_mutex = PTHREAD_MUTEX_INITIALIZER;

static int _sigpipe_h_installed = 0;
static int _sigpipe_h_received = 0;

static void _sigpipe_h(int signal)
{
  _sigpipe_h_received++; 
}


/* ---------------------------------------------------------------------------*/

int rcs_init(const int *portid)
{
  struct sockaddr_in  srvaddr;
  struct hostent     *srv = NULL;
  char   srvid[HOST_NAME_MAX+1];
  char   ip[16];

  if (srvhdl != -1) { return -1; }

  /* retrieve and set server information */
  gethostname(srvid,sizeof(srvid));
  if ((srv = gethostbyname(srvid)) == NULL) {
    fprintf(stderr,"%s: %s\n",hstrerror(h_errno),srvid);
    return -1; 
  }
  sprintf(ip, "%u.%u.%u.%u",
	  ((unsigned char *)srv->h_addr)[0],
	  ((unsigned char *)srv->h_addr)[1],
	  ((unsigned char *)srv->h_addr)[2],
	  ((unsigned char *)srv->h_addr)[3]);
  srvaddr.sin_family = AF_INET;
  if (!portid) { srvaddr.sin_port = htons(PORT); }
  else { srvaddr.sin_port = htons(*portid); }
  if (inet_aton(ip,&srvaddr.sin_addr) == 0) {
    fprintf(stderr,"server address not valid : %s\n",inet_ntoa(srvaddr.sin_addr));
    return -1;
  }

  /* create socket */
  srvhdl=socket(PF_INET, SOCK_STREAM, 0);
  if (srvhdl==-1) {
    perror("socket error");
    return -1;
  }

  /* bind socket */
  if (bind(srvhdl,(struct sockaddr*)&srvaddr,sizeof(srvaddr))) {
    perror("bind error");
    close(srvhdl);
    return -1;
  }

  /* install signal handler */
  if (!_sigpipe_h_installed) {
    signal(SIGPIPE,_sigpipe_h);
  }

  /* listen on socket */
  if (listen(srvhdl,0) < 0) {
    perror("listen error");
    close(srvhdl);
    return -1;
  }

  connects=0;
  
  return 0;
}


/* ---------------------------------------------------------------------------*/

int rcs_term()
{
  if (srvhdl != -1) {
    close(srvhdl);
    srvhdl = -1;
    _sigpipe_h_installed=0;
    return 0;
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/

int rcs_accept(int *clthdl)
{
  struct linger optval = {1};
  socklen_t optlen = sizeof(optval);

  pthread_mutex_lock(&connect_mutex);
  if (clthdl) {
    *clthdl=accept(srvhdl,NULL,0);
    if (*clthdl == -1) {
      perror("rcs accept");
      pthread_mutex_unlock(&connect_mutex);
      return -1;
    }

    if (setsockopt(*clthdl,SOL_SOCKET,SO_LINGER,(void*)&optval,optlen) != 0) {
      perror("setsockopt SO_LINGER");
    }
    
    connects++;
    pthread_mutex_unlock(&connect_mutex);
    return 0;
  }
  pthread_mutex_unlock(&connect_mutex);
  return -1;
}


/* ---------------------------------------------------------------------------*/

int rcs_getrequest(int clthdl, void *reqdata, size_t *reqdatalen)
{
  struct pollfd pf;
  size_t readlen=0;
  size_t recvlen=0;
  int    maxlen =0;

  if ((srvhdl != -1) && (clthdl > 0) && reqdata && reqdatalen) {

    memset(reqdata,0,*reqdatalen);
    pf.fd = clthdl;
    pf.events=POLLIN;

    if ((poll(&pf,1,1000) == 1 ) && (pf.revents & (POLLIN))) {
      if (*reqdatalen>0) maxlen=*reqdatalen;
      do {
	/* get length */
	readlen=read(clthdl,(void*)reqdatalen+recvlen,sizeof(size_t)-recvlen);
	if (readlen <= 0) break;
	recvlen += readlen;
      } while (recvlen != sizeof(size_t));

      if (maxlen > 0 && *reqdatalen > maxlen) {
	fprintf(stderr,
		"getrequest buffer to small, needed %d available %d\n",
		*reqdatalen,
		readlen);
      } else if (readlen > 0) {
	readlen=0;
	recvlen=0;
	do {
	  /* get data */
	  readlen=read(clthdl,reqdata+recvlen,*reqdatalen-recvlen);
	  if (readlen <= 0) break;
	  recvlen += readlen;
	} while (recvlen != *reqdatalen);
	
	if (readlen > 0) { return 0; }
      }
    }
    close(clthdl);
    return -1;
  }

  if (clthdl > 0) { close(clthdl); }
  perror("rcserv request");
  return -1;
}


/* ---------------------------------------------------------------------------*/
/*                              end of rcserv_ip.c                            */
/* ---------------------------------------------------------------------------*/
