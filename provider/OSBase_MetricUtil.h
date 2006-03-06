/*
 * $Id: OSBase_MetricUtil.h,v 1.9 2006/03/06 11:45:05 mihajlov Exp $
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
 * Description: Metric Provider Utility Functions
 */

#ifndef OSBASE_METRICUTIL_H
#define OSBASE_METRICUTIL_H

#include <time.h>
#include <cmpidt.h>
#include <repos.h>

int checkRepositoryConnection();

/* maintain cache of CIM class to id mappings */
int refreshMetricDefClasses(CMPIBroker *broker, CMPIContext *ctx, 
			    const char *namesp, int force);
void releaseMetricDefClasses();

/* retrieve CIM class name for given ids */
int metricDefClassName(CMPIBroker *broker, CMPIContext *ctx, 
		       const char *namesp, char *clsname, 
		       const char *name, int id);
int metricValueClassName(CMPIBroker *broker, CMPIContext *ctx, 
			 const char *namesp,char *clsname, 
			 const char *name, int id);
int metricPluginName(CMPIBroker *broker, CMPIContext *ctx, 
		     const char *namesp,char *pluginname, 
		     const char *name, int id);

/* metric id to CIM key mappings */
char * makeMetricDefIdFromCache(CMPIBroker *broker, CMPIContext *ctx,
				const char * namesp, char * defid, int id);
char * makeMetricValueIdFromCache(CMPIBroker *broker, CMPIContext *ctx,
				  const char * namesp, char * valid, int id,
				  const char * resource, const char * systemid,
				  time_t timestamp);
char * makeMetricDefId(char * defid, const char * name, int id);
int parseMetricDefId(const char * defid,
		     char * name, int * id);
char * makeMetricValueId(char * instid, const char * name, int id, 
			 const char * resource, const char * systemid,
			 time_t timestamp);
int parseMetricValueId(const char * instid,
		       char * name, int * id, char * resource,
		       char * systemid, time_t * timestamp);

/* plugin name for metric definition class name */
int getPluginNamesForValueClass(CMPIBroker *broker, CMPIContext *ctx, 
				CMPIObjectPath *cop, char ***pluginnames);
void releasePluginNames(char **pluginnames);
int getMetricDefsForClass(CMPIBroker *broker, CMPIContext *ctx, 
			  CMPIObjectPath* cop,
			  char ***mnames, int **mids);
void releaseMetricDefs(char **mnames,int *mids); 

/* resource class to metric id mapping */
int getMetricIdsForResourceClass(CMPIBroker *broker, CMPIContext *ctx, 
				 CMPIObjectPath* cop,
				 char ***metricnames,
				 int **mids, 
				 char ***resourceids,
				 char ***systemids);
void releaseMetricIds(char **metricnames, int *mids, char **resourceids,
		      char **systemids); 

/* support for instance and object path construction */
CMPIString * val2string(CMPIBroker * broker, const ValueItem *val,
			unsigned   datatype);
CMPIInstance * makeMetricValueInst(CMPIBroker * broker, 
				   CMPIContext * ctx,
				   const char * defname,
				   int defid,
				   const ValueItem *val,
				   unsigned   datatype,
				   CMPIObjectPath *cop,
				   CMPIStatus * rc);
CMPIObjectPath * makeMetricValuePath(CMPIBroker * broker,
				     CMPIContext * ctx,
				     const char * defname,
				     int defid,
				     const ValueItem *val, 
				     CMPIObjectPath *cop,
				     CMPIStatus * rc);

CMPIObjectPath * makeMetricDefPath(CMPIBroker * broker,
				   CMPIContext * ctx,
				   const char * defname,
				   int defid,
				   const char * namesp,
				   CMPIStatus * rc);
CMPIInstance * makeMetricDefInst(CMPIBroker * broker,
				 CMPIContext * ctx,
				 const char * defname,
				 int defid,
				 const char * namesp,
				 CMPIStatus * rc);

CMPIObjectPath * makeResourcePath(CMPIBroker * broker,
				  CMPIContext * ctx,
				  const char * namesp,
				  const char * defname,
				  int defid,
				  const char * resourcename,
				  const char * systemid);

void computeResourceNamespace(CMPIObjectPath *rescop,
			      CMPIObjectPath *mcop,
			      const char *systemid);

#endif
