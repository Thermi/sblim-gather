/*
 * $Id: rplugmgr.c,v 1.1 2004/07/16 15:30:05 mihajlov Exp $
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
 * Description: Repository Plugin Manager
 * Does dynamic loading of repository plugins.
 */

#include "rplugmgr.h"
#include "rplugin.h"
#include <dlfcn.h>
#include <stdio.h>

int RP_Load (RepositoryPlugin *plugin)
{
  RepositoryMetricsDefined  *mdef;

  if (plugin==NULL || plugin->rpName==NULL || plugin->rpName[0]==0) {
    fprintf(stderr,"Null plugin name\n");
    return -1;
  }
  plugin->rpHandle = dlopen(plugin->rpName,RTLD_LAZY);
  if (!plugin->rpHandle) {
    fprintf(stderr,"Error loading plugin library %s: %s\n", plugin->rpName, 
	    dlerror());
    return -1;
  }
  mdef = dlsym(plugin->rpHandle,REPOSITORYMETRIC_DEFINITIONPROC_S);
  if (!mdef) {
    fprintf(stderr,
	    "Error locating " REPOSITORYMETRIC_DEFINITIONPROC_S " in %s: %s\n", 
	    plugin->rpName,dlerror());
    dlclose(plugin->rpHandle);
    return -1;
  }
  /* clean out some fields, then get metrics */
  plugin->rpNumMetricCalcDefs=0;
  plugin->rpMetricCalcDefs=NULL;
  if (mdef(plugin->rpRegister,
	   plugin->rpName,
	   &plugin->rpNumMetricCalcDefs,
	   &plugin->rpMetricCalcDefs)) {
    fprintf(stderr,"Couldn't get metrics for repository plugin %s\n",
	    plugin->rpName); 
    dlclose(plugin->rpHandle);
    return -1;
  }
  return 0;
}

int RP_Unload(RepositoryPlugin *plugin)
{  
  if (plugin==NULL || plugin->rpName==NULL || plugin->rpHandle==NULL) {
    fprintf(stderr,"Null plugin specified\n");
    return -1;
  }
  if (dlclose(plugin->rpHandle)) {
    fprintf(stderr,"Couldn't unload plugin\n");
    return -1;
  }
  plugin->rpHandle=NULL; /* disallow accidental reuse */
  return 0;
}
