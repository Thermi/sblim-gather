/*
 * $Id: repos.h,v 1.2 2004/07/16 15:30:04 mihajlov Exp $
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
 * Description: Local Repository Interface
 */

#ifndef REPOS_H
#define REPOS_H

#include <stdlib.h>
#include "commheap.h"

#ifdef __cplusplus
extern "C" {
#endif

int repos_init(); 
int repos_terminate(); 

int reposplugin_add(const char *pluginname);
int reposplugin_remove(const char *pluginname);

typedef struct _RepositoryPluginDefinition {
  int      rdId;
  unsigned rdDataType;
  char    *rdName; 
  char   **rdResource; 
} RepositoryPluginDefinition;

int reposplugin_list(const char *pluginname,
		     RepositoryPluginDefinition **pdef, 
		     COMMHEAP ch);

typedef struct _RepositoryStatus {
  short    rsInitialized;
  unsigned rsNumPlugins;
  unsigned rsNumMetrics;
} RepositoryStatus;

void repos_status(RepositoryStatus *rs); 

typedef struct _ValueItem {
  time_t viCaptureTime;
  time_t viDuration;
  size_t viValueLen;
  char * viValue;
  char * viResource;
} ValueItem;

typedef struct _ValueRequest {
  int         vsId;
  char       *vsResource;
  time_t      vsFrom;
  time_t      vsTo;
  unsigned    vsDataType;
  int         vsNumValues;
  ValueItem  *vsValues;
} ValueRequest;

typedef void * MVENUM;

int reposvalue_put(ValueRequest *vs, COMMHEAP ch);
int reposvalue_get(ValueRequest *vs, COMMHEAP ch);

#ifdef __cplusplus
}
#endif

#endif
