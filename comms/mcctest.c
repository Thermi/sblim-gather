/*
 * $Id: mcctest.c,v 1.2 2004/07/16 15:30:05 mihajlov Exp $
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
 * Description: Client Side Test Program
 */

#include "mcclt.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

static int transactions = 0;



int main()
{
  MC_REQHDR  req;
  char   buf[500];
  size_t buflen;
  time_t start, end;

  start=time(NULL);
  mcc_init("mcstest");

  do {
    strcpy(buf,"x");
    /* fgets(buf,sizeof(buf),stdin); */
    if (*buf=='q') {
      req.mc_type=0;
      mcc_request(&req,"",0);
      break;
    } else {
      req.mc_type=1;
      buflen=sizeof(buf);
      if (mcc_request(&req,buf,strlen(buf))==0) {
	mcc_response(&req,buf,&buflen);	
	if (transactions++ == 500000) {
	  end = time(NULL);
	  fprintf(stderr,"Transactions %d in seconds %ld, rate = %ld\n",
		  transactions,end-start,transactions/(end-start));
	  break;
	}
	/*puts(buf);*/
      } else {
	break;
      }
    }
  } while(1);

  mcc_term();
  return 0;
}
