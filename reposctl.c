/*
 * $Id: reposctl.c,v 1.3 2004/08/03 12:36:08 mihajlov Exp $
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
 * Author:       Viktor Mihajlovski <mihajlov@de.ibm.cim>
 * Contributors: 
 *
 * Description:  Repository Remote Controller
 * Command line interface to the repository daemon.
 *  
 */

#include "metric.h"
#include "rrepos.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static const char* commands[] = {
  "\th\t\tprint this help message\n",
  "\ts\t\tstatus\n",
  "\ti\t\tinit\n",
  "\tt\t\tterminate\n",
  "\tl plugin\tload plugin\n",
  "\tu plugin\tunload plugin\n",
  "\tv plugin\tview/list metrics for plugin\n",
  "\tq\t\tquit\n",
  "\tg id [resource [from [to]]]\tget metric value\n",
  NULL
};

static void printhelp();

int main()
{
  char              cmd;
  char             *arg;
  char              buf[500];
  char              argbuf[500];
  int               quit=0;
  int               i, j;
  RepositoryStatus  rs;
  int               pnum;
  COMMHEAP          commh;
  int               offFrom, offTo;
  ValueRequest      vr;
  RepositoryPluginDefinition *rdef;
  
  while(fgets(buf,sizeof(buf),stdin)) {
    cmd = buf[0];
    arg = buf+1;
    switch(cmd) {
    case 'h':
      printhelp();
      break;
    case 's':
      if (rrepos_status(&rs) == 0) {
	printf("Status %sinitialized," 
	       " %d plugins and %d metrics. \n",
	       rs.rsInitialized?"":"NOT ",
	       rs.rsNumPlugins, rs.rsNumMetrics);
      } else {
	printf("Daemon not reachable.\n");
      }
      break;
    case 'i':
      if(rrepos_init())
	printf("Failed\n");
      break;
    case 't':
      if(rrepos_terminate())
	printf("Failed\n");
      break;
    case 'l':
      sscanf(arg,"%s",argbuf);
      if (rreposplugin_add(argbuf))
	printf("Failed\n");
      break;
    case 'u':
      sscanf(arg,"%s",argbuf);
      if (rreposplugin_remove(argbuf))
	printf("Failed\n");
      break;
    case 'v':
      commh=ch_init();
      sscanf(arg,"%s",argbuf);
      pnum=rreposplugin_list(argbuf,&rdef,commh);
      if (pnum < 0) {
	printf("Failed\n");
      } else {
	for (i=0;i<pnum;i++) {
	  printf("Plugin metric \"%s\" has id %d and data type %x\n",
		 rdef[i].rdName, rdef[i].rdId, rdef[i].rdDataType);
	  for (j=0;rdef[i].rdResource[j];j++) {
	    printf("\t for resource \"%s\"\n",rdef[i].rdResource[j]);
	  }
	}
      }
      ch_release(commh);
      break;
    case 'g':
      vr.vsId = 0;
      offFrom = 0;
      offTo  = 0;
      vr.vsResource = argbuf;
      vr.vsResource[0]=0;
      sscanf(arg,"%d %s %d %d",&vr.vsId,vr.vsResource,&offFrom,&offTo);
      if (strlen(vr.vsResource)==0 || vr.vsResource[0]=='*') 
	vr.vsResource=NULL;
      if (offTo) 
	vr.vsTo = time(NULL) + offTo;
      else
	vr.vsTo = 0;
      if (offFrom) 
	vr.vsFrom = time(NULL) + offFrom;
      else
	vr.vsFrom = 0;
      commh=ch_init();
      if (rrepos_get(&vr,commh)) {
	printf("Failed\n");
      } else {
	printf("Value id %d has value data \n",
	       vr.vsId);
	for (i=0; i < vr.vsNumValues; i++) {
	  printf("\t for resource %s ",vr.vsValues[i].viResource);
	  switch(vr.vsDataType) {
	  case MD_BOOL:
	    printf("%s", *vr.vsValues[i].viValue ? "true" : "false" );
	    break;
	  case MD_SINT8:
	    printf("%hhd",*vr.vsValues[i].viValue);
	    break;
	  case MD_UINT8:
	    printf("%hhu",*(unsigned char*)vr.vsValues[i].viValue);
	    break;
	  case MD_UINT16:
	  case MD_CHAR16:
	    printf("%hu",*(unsigned short*)vr.vsValues[i].viValue);
	    break;
	  case MD_SINT16:
	    printf("%hd",*(short*)vr.vsValues[i].viValue);
	    break;
	  case MD_UINT32:
	    printf("%lu",*(unsigned long*)vr.vsValues[i].viValue);
	    break;
	  case MD_SINT32:
	    printf("%ld",*(long*)vr.vsValues[i].viValue);
	    break;
	  case MD_UINT64:
	    printf("%llu",*(unsigned long long*)vr.vsValues[i].viValue);
	    break;
	  case MD_SINT64:
	    printf("%lld",*(long long*)vr.vsValues[i].viValue);
	    break;
	  case MD_FLOAT32:
	    printf("%f",*(float*)vr.vsValues[i].viValue);
	    break;
	  case MD_FLOAT64:
	    printf("%f",*(double*)vr.vsValues[i].viValue);
	    break;
	  case MD_STRING:
	    printf(vr.vsValues[i].viValue);
	    break;
	  default:
	    printf("datatype %0x not supported",vr.vsDataType);
	    break;
	  }
	  printf(", sample time %ld duration %ld",
		 vr.vsValues[i].viCaptureTime,
		 vr.vsValues[i].viDuration);
	  printf ("\n");
	}
      }
      ch_release(commh);
      break;
    case 'q':
      quit=1;
      break;
    default:
      printhelp();
      break;
    }
    if (quit) break;
  }
  
  return 0;
}

static void printhelp()
{
  int i;
  for (i=0;commands[i];i++)
    printf(commands[i]);
}
