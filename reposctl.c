/*
 * $Id: reposctl.c,v 1.1 2004/07/16 15:30:04 mihajlov Exp $
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
