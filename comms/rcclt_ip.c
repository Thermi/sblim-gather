/*
 * $Id: rcclt_ip.c,v 1.2 2004/10/14 15:54:25 heidineu Exp $
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
 * Contributors: Heidi Neumann <heidineu@de.ibm.com>
 *
 * Description:  Gatherer Client-Side Communication APIs
 *               TCP/IP Socket version
 *
 */

#include "rcclt.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


/* ---------------------------------------------------------------------------*/

static struct sockaddr_in srvaddr;
static int srvhdl;
static int connects;
static int requests;

static int _sigpipe_h_installed = 0;
static int _sigpipe_h_received = 0;

static void _sigpipe_h(int signal)
{
  _sigpipe_h_received++;
}


/* ---------------------------------------------------------------------------*/

int rcc_init(const char *srvid, const int *portid)
{
  struct hostent *srv = NULL;
  char   ip[16];

  if (srvid && portid && !srvaddr.sin_family) {

    /* retrieve and set server information */
    if ((srv = gethostbyname(srvid)) == NULL) {
      fprintf(stderr,"rcclient %s: %s\n",hstrerror(h_errno),srvid);
      return -1; 
    }
    sprintf(ip, "%u.%u.%u.%u",
	    ((unsigned char *)srv->h_addr)[0],
	    ((unsigned char *)srv->h_addr)[1],
	    ((unsigned char *)srv->h_addr)[2],
	    ((unsigned char *)srv->h_addr)[3]);
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port   = htons(*portid);
    if (inet_aton(ip,&srvaddr.sin_addr) == 0) {
      fprintf(stderr,"rcclient server address not valid : %s\n",inet_ntoa(srvaddr.sin_addr));
      return -1;
    }

    /* install signal handler */
    if (!_sigpipe_h_installed) {
      signal(SIGPIPE,_sigpipe_h);
    }

    /* init global variables */
    srvhdl=-1;
    connects=0;
    requests=0;

    return 0;
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/

int rcc_term()
{
  if (srvhdl > 0) {
    close(srvhdl);
    srvhdl = -1;
    memset(&srvaddr,0,sizeof(struct sockaddr_in));
    return 0;
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/

int _rcc_connect() {

  if (srvhdl > 0) { close(srvhdl); }

  srvhdl=socket(PF_INET, SOCK_STREAM, 0);
  if (srvhdl==-1) {
    perror("rcclient socket error");
    return -1;
  }
  if (connect(srvhdl,(struct sockaddr*)&srvaddr,sizeof(struct sockaddr_in)) < 0) {
    perror("rcclient connect error");
    return -1;
  }
  connects++;
  return 0;
}

/* ---------------------------------------------------------------------------*/

int rcc_request(void *reqdata, size_t reqdatalen)
{
  size_t sentlen;
  size_t reqlen;

  if (reqdata == NULL) return -1;
  
  if (srvhdl <= 0) {
    if (_rcc_connect() < 0) {
      perror("rcclient connect");
      return -1;
    }
  }

  reqlen = reqdatalen+sizeof(size_t);

  sentlen = write(srvhdl,&reqdatalen,sizeof(size_t)) +
    write(srvhdl,reqdata,reqdatalen);

  if (errno == EPIPE) {
    if (_rcc_connect() < 0) {
      perror("rcclient reconnect");
      return -1;
    }
    else {
      sentlen = write(srvhdl,&reqdatalen,sizeof(size_t)) +
	write(srvhdl,reqdata,reqdatalen);
    }
  }

  if (sentlen == reqlen) { 
    requests++;
    return 0; 
  }

  fprintf(stderr,"rcclient sendrequest error, wanted %d got %d\n",
	  reqdatalen+sizeof(size_t), sentlen);
  perror("rcclient send");
  return -1;
}


/* ---------------------------------------------------------------------------*/
/*                             end of rcclt_ip.c                              */
/* ---------------------------------------------------------------------------*/
