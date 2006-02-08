/*
 * $Id: rcctest.c,v 1.5 2006/02/08 20:50:57 mihajlov Exp $
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
 * Description: Client Side Test Program
 *              TCP/IP Socket version
 */

#include "rcclt.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

static int transactions = 0;

/* ---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
  char   hostname[256];
  char   buf[500];
  size_t buflen;
  time_t start, end;
  int    port = 6363;
  int    i    = 0;

  start=time(NULL);

  if (argc == 1) {
    gethostname((char*)&hostname,sizeof(hostname));
  } else {
    sprintf(hostname,argv[1]);
  }
  printf("Contacting %s\n",hostname);
  if (rcc_init(hostname,&port) < 0 ) {
    return -1;
  }

  do {
    sprintf(buf,"integer value %i",i);
    i++;
    //fprintf(stdout,"rcctest>");
    //fgets(buf,sizeof(buf),stdin);
    if (*buf=='q') {
      rcc_request("",0);
      break;
    } else {
      buflen=sizeof(buf);
      if (rcc_request(buf,strlen(buf)+1)==0) {
	fprintf(stderr,"send : %s\n",buf);
      }
      else { break; }
      //sleep(3);
      if (transactions++ == 10) {
	end = time(NULL);
	fprintf(stderr,"Transactions %d in seconds %ld, rate = %ld\n",
		transactions,end-start,end-start>0?transactions/(end-start):-1);
	break;
      }
    }
  } while(1);

  rcc_term();
  return 0;
}


/* ---------------------------------------------------------------------------*/
/*                              end of rcctest.c                              */
/* ---------------------------------------------------------------------------*/
