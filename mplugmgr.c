/*
 * $Id: mplugmgr.c,v 1.1 2003/10/17 13:56:01 mihajlov Exp $
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
 * Description: Metric Plugin Manager
 * Does dynamic loading of metric plugins.
 */

#include "mplugmgr.h"
#include "mplugin.h"
#include <dlfcn.h>
#include <stdio.h>

int MP_Load (MetricPlugin *plugin)
{
  MetricsDefined  *mdef;
  MetricStartStop *mss;

  if (plugin==NULL || plugin->mpName==NULL || plugin->mpName[0]==0) {
    fprintf(stderr,"Null plugin name\n");
    return -1;
  }
  plugin->mpHandle = dlopen(plugin->mpName,RTLD_LAZY);
  if (!plugin->mpHandle) {
    fprintf(stderr,"Error loading plugin library %s: %s\n", plugin->mpName, 
	    dlerror());
    return -1;
  }
  mdef = dlsym(plugin->mpHandle,METRIC_DEFINITIONPROC_S);
  if (!mdef) {
    fprintf(stderr,"Error locating " METRIC_DEFINITIONPROC_S " in %s: %s\n", 
	    plugin->mpName,dlerror());
    dlclose(plugin->mpHandle);
    return -1;
  }
  mss = dlsym(plugin->mpHandle,METRIC_STARTSTOPPROC_S);
  if(!mss) {
    fprintf(stderr,"Error locating " METRIC_STARTSTOPPROC_S " in %s: %s\n", 
	    plugin->mpName, dlerror());
    dlclose(plugin->mpHandle);
    return -1;
  }
  /* clean out some fields, then get metrics */
  plugin->mpNumMetricDefs=0;
  plugin->mpMetricDefs=NULL;
  if (mdef(plugin->mpRegister,
	   plugin->mpName,
	   &plugin->mpNumMetricDefs,
	   &plugin->mpMetricDefs)) {
    fprintf(stderr,"Couldn't get metrics for plugin %s\n",plugin->mpName); 
    dlclose(plugin->mpHandle);
    return -1;
  }
  /* Start plugin */
  if (mss(1)) {
    fprintf(stderr,"Couldn't start plugin %s\n",plugin->mpName); 
    dlclose(plugin->mpHandle);
    return -1;
  }
  return 0;
}

int MP_Unload(MetricPlugin *plugin)
{
  MetricStartStop *mss;
  
  if (plugin==NULL || plugin->mpName==NULL || plugin->mpHandle==NULL) {
    fprintf(stderr,"Null plugin specified\n");
    return -1;
  }
  mss = dlsym(plugin->mpHandle,METRIC_STARTSTOPPROC_S);
  if(!mss) {
    fprintf(stderr,"Error locating " METRIC_STARTSTOPPROC_S " in %s: %s\n", 
	    plugin->mpName,dlerror());
    return -1;
  }
  /* Stop plugin */
  if (mss(0)) {
    fprintf(stderr,"Couldn't stop plugin - forcing unload\n"); 
  }
  if (dlclose(plugin->mpHandle)) {
    fprintf(stderr,"Couldn't unload plugin\n");
    return -1;
  }
  plugin->mpHandle=NULL; /* disallow accidental reuse */
  return 0;
}
