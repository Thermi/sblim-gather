/*
 * $Id: rgather.h,v 1.1 2003/10/17 13:56:01 mihajlov Exp $
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
 * Description: Remote Gatherer API
 */

#ifndef RGATHER_H
#define RGATHER_H


#ifdef __cplusplus
extern "C" {
#endif

#include "gather.h"
#include "commheap.h"

int rgather_load();
int rgather_unload();

int rgather_init(); 
int rgather_start(); 
int rgather_stop(); 
int rgather_terminate(); 

int rmetricplugin_add(const char *pluginname);
int rmetricplugin_remove(const char *pluginname);
int rmetricplugin_list(const char *pluginname, PluginDefinition **pdef, 
		       COMMHEAP ch);
  
int rgather_status(GatherStatus *gs); 
int rgather_get(ValueRequest *vr, COMMHEAP ch);

#ifdef __cplusplus
}
#endif

#endif
