/*
 * $Id: mcstest.c,v 1.1 2003/10/17 13:56:01 mihajlov Exp $
 *
 * (C) Copyright IBM Corp. 2003
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
 * Description: Server Side Communciation Test 
 * 
 */

#include "mcserv.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main()
{
  MC_REQHDR  req;
  char       buf[500];
  size_t     buflen;
  
  if (mcs_init() == 0) {
    do {
      buflen=sizeof(buf);
      if (mcs_getrequest(&req,buf,&buflen) == -1) {
	break;
      }
      /*      if (req.mc_len>0)
	      puts(req.mc_data);*/
      if (req.mc_type!=0) {
	buflen = 5;
	strcpy(buf,"JUHU");
	if (mcs_sendresponse(&req,buf,buflen) == -1) {
	  break;
	}
      } else {
	break;
      }
    } while (1);
    mcs_term();
  }  
  return 0;
}


