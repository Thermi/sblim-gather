/*
 * $Id: gatherctl.c,v 1.2 2004/07/16 15:30:04 mihajlov Exp $
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
 * Description:  Gatherer Remote Controller
 * Command line interface to the gatherer daemon.
 *  
 */

#include "metric.h"
#include "rgather.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static const char* commands[] = {
  "\th\t\tprint this help message\n",
  "\ts\t\tstatus\n",
  "\ti\t\tinit\n",
  "\tt\t\tterminate\n",
  "\tb\t\tstart sampling\n",
  "\te\t\tstop sampling\n",
  "\tl plugin\tload plugin\n",
  "\tu plugin\tunload plugin\n",
  "\tv plugin\tview/list metrics for plugin\n",
  "\tq\t\tquit\n",
  "\tk\t\tkill daemon\n",
  "\td\t\tstart daemon\n",
  NULL
};

static void printhelp();

int main()
{
  char   cmd;
  char  *arg;
  char   buf[500];
  char   argbuf[500];
  int    quit=0;
  int    i, j;
  GatherStatus gs;
  PluginDefinition *pdef;
  int              pnum;
  COMMHEAP commh;
  
  while(fgets(buf,sizeof(buf),stdin)) {
    cmd = buf[0];
    arg = buf+1;
    switch(cmd) {
    case 'h':
      printhelp();
      break;
    case 's':
      if (rgather_status(&gs) == 0) {
	printf("Status %sinitialized and %ssampling," 
	       " %d plugins and %d metrics. \n",
	       gs.gsInitialized?"":"NOT ",
	       gs.gsSampling?"":"NOT ",
	       gs.gsNumPlugins, gs.gsNumMetrics);
      } else {
	printf("Daemon not reachable.\n");
      }
      break;
    case 'i':
      if(rgather_init())
	printf("Failed\n");
      break;
    case 't':
      if(rgather_terminate())
	printf("Failed\n");
      break;
    case 'b':
      if(rgather_start())
	printf("Failed\n");
      break;
    case 'e':
      if(rgather_stop())
	printf("Failed\n");
      break;
    case 'l':
      sscanf(arg,"%s",argbuf);
      if (rmetricplugin_add(argbuf))
	printf("Failed\n");
      break;
    case 'u':
      sscanf(arg,"%s",argbuf);
      if (rmetricplugin_remove(argbuf))
	printf("Failed\n");
      break;
    case 'v':
      commh=ch_init();
      sscanf(arg,"%s",argbuf);
      pnum=rmetricplugin_list(argbuf,&pdef,commh);
      if (pnum < 0) {
	printf("Failed\n");
      } else {
	for (i=0;i<pnum;i++) {
	  printf("Plugin metric \"%s\" has id %d and data type %x\n",
		 pdef[i].pdName, pdef[i].pdId, pdef[i].pdDataType);
	  for (j=0;pdef[i].pdResource[j];j++) {
	    printf("\t for resource \"%s\"\n",pdef[i].pdResource[j]);
	  }
	}
      }
      ch_release(commh);
      break;
    case 'q':
      quit=1;
      break;
    case 'k':
      if(rgather_unload())
	printf("Failed to unload daemon\n");
      break;
    case 'd':
      if(rgather_load())
	printf("Failed to load daemon\n");
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
