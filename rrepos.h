/*
 * $Id: rrepos.h,v 1.5 2004/11/10 16:08:24 heidineu Exp $
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
 * Description: Remote Repository API
 */

#ifndef RREPOS_H
#define RREPOS_H


#ifdef __cplusplus
extern "C" {
#endif

#include "commheap.h"
#include "metric.h"
#include "repos.h"
#include "gather.h"

/*
 * Session check, meaning of return codes
 * 0  Session OK
 * 1  Session newly established
 * -1 Session cannot be established
 */
int rrepos_sessioncheck();

/*
 * Register Plugin Definition, meaning of return codes
 * 0  Successfully registered
 * -1 Could not register
 */
int rrepos_register(const PluginDefinition *pdef);

/*
 * Store value in remote repository
 * 0  OK
 * -1 Failure
 */
int rrepos_put(const char *reposplugin, const char *metric,MetricValue *mv);

/*
 * Retrieve value from remote repository
 * 0  OK
 * -1 Failure
 */
int rrepos_get(ValueRequest *vs, COMMHEAP ch);

/*
 * Control interface, spawn repository daemon
 */
int rrepos_load();

/*
 * Control interface, kill repository daemon
 */
int rrepos_unload();

/*
 * Control interface, initialize repository daemon
 */
int rrepos_init(); 

/*
 * Control interface, terminate repository daemon
 */
int rrepos_terminate(); 

/*
 * Control interface, return status of the repository daemon
 */
int rrepos_status(RepositoryStatus *rs); 

/*
 * Control interface, load a repository plugin
 */
int rreposplugin_add(const char *pluginname);

/*
 * Control interface, remove a repository plugin
 */
int rreposplugin_remove(const char *pluginname);

/*
 * Control interface, list repository plugin definitions
 */
int rreposplugin_list(const char *pluginname,
		      RepositoryPluginDefinition **pdef, 
		      COMMHEAP ch);





/*
 * Retrieve resource list from remote repository
 * 0  OK
 * -1 Failure
 */
int rreposresource_list(const char * metricid,
			MetricResourceId **rid, 
			COMMHEAP ch);


 /*
  * dummy for testing
  */
int rrepos_subscribe(char *id);

#ifdef __cplusplus
}
#endif

#endif
