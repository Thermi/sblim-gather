/*
 * $Id: rcstest.c,v 1.2 2004/10/14 16:28:50 heidineu Exp $
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
 * Description: Server Side Communciation Test 
 *              TCP/IP Socket version
 * 
 */

#include "rcserv.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAXCONN 10

static int clthdl[MAXCONN] = {0};
static pthread_t thread_id[MAXCONN];
static int connects = 0;
static pthread_mutex_t connect_mutex = PTHREAD_MUTEX_INITIALIZER;


/* ---------------------------------------------------------------------------*/

static void * _get_request(void * hdl)
{
  int    i = 0;
  char   buf[500];
  size_t buflen;

  fprintf(stderr,"--- start thread on socket %i\n",(int)hdl);
  
  while (1) {
    buflen = sizeof(buf);
    if (rcs_getrequest((int)hdl,buf,&buflen) == -1) {
      fprintf(stderr,"--- time out on socket %i\n",(int)hdl);
      break;
    }
    fprintf(stderr,"---- received on socket %i: %s\n",(int)hdl,buf);
  }

  pthread_mutex_lock(&connect_mutex);
  for(i=0;i<MAXCONN;i++) {
    if (clthdl[i] == (int)hdl) {
      clthdl[i] = -1;
      connects--;
      break;
    }
  }
  pthread_mutex_unlock(&connect_mutex);
  fprintf(stderr,"--- exit thread on socket %i\n",(int)hdl);
  return NULL;
}


/* ---------------------------------------------------------------------------*/

int main()
{
  int hdl  = -1;
  int port = 6363;
  int i    = 0;

  memset(thread_id,0,sizeof(thread_id));

  if (rcs_init(&port) == 0) {
    do {

      if (hdl == -1) {
	if (rcs_accept(&hdl) == -1) {
	  return -1;
	}
      }

      pthread_mutex_lock(&connect_mutex);

      for(i=0;i<MAXCONN;i++) { 
	if (clthdl[i] <= 0) {
	  clthdl[i] = hdl;
	  break;
	} 
      }
      
      if (pthread_create(&thread_id[i],NULL,_get_request,(void*)hdl) != 0) {
	perror("create thread");
	return -1;
      }

      hdl = -1;
      fprintf(stderr,"thread_id [%i] : %ld\n",i,thread_id[i]);
      if (connects<MAXCONN) { connects++; }
      else { 
	pthread_mutex_unlock(&connect_mutex);
	break; 
      }
      pthread_mutex_unlock(&connect_mutex);
    
    } while (1);
    rcs_term();
  }  
  return 0;
}


/* ---------------------------------------------------------------------------*/
/*                              end of rcstest.c                              */
/* ---------------------------------------------------------------------------*/
