/*
 * $Id: OSBase_MetricDefForMEProvider.c,v 1.1 2004/11/10 16:08:24 heidineu Exp $
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
 * Author:       Heidi Neumann <heidineu@de.ibm.com>
 *
 * Interface Type : Common Manageability Programming Interface ( CMPI )
 *
 * Description: Association Provider for CIM_MetricDefForMe
 * 
 */


#include <cmpidt.h>
#include <cmpift.h>
#include <cmpimacs.h>
#include <rrepos.h>
#include "OSBase_MetricUtil.h"

#define LOCALCLASSNAME "Linux_MetricDefForME"

static CMPIBroker * _broker;

/* ------------------------------------------------------------------ *
 * Simplified Utility Functions
 * no support for assocClass, resultClass, etc.
 * ------------------------------------------------------------------ */

static CMPIInstance * _makeRefInstance(const CMPIObjectPath *defp,
				       const CMPIObjectPath *valp)
{
  CMPIObjectPath *co = CMNewObjectPath(_broker,NULL,LOCALCLASSNAME,NULL);
  CMPIInstance   *ci = NULL;
  if (co) {
    CMSetNameSpaceFromObjectPath(co,defp);
    ci = CMNewInstance(_broker,co, NULL);
    if (ci) {
      CMSetProperty(ci,"Antecedent",&defp,CMPI_ref);
      CMSetProperty(ci,"Dependent",&valp,CMPI_ref);
    }
  }
  return ci;
}

static CMPIObjectPath * _makeRefPath(const CMPIObjectPath *defp,
				       const CMPIObjectPath *valp)
{
  CMPIObjectPath *co = CMNewObjectPath(_broker,NULL,LOCALCLASSNAME,NULL);
  if (co) {
    CMSetNameSpaceFromObjectPath(co,defp);
    CMAddKey(co,"Antecedent",&defp,CMPI_ref);
    CMAddKey(co,"Dependent",&valp,CMPI_ref);
  }
  return co;
}

static CMPIStatus associatorHelper( CMPIResult * rslt,
				    CMPIContext * ctx,
				    CMPIObjectPath * cop,
				    int associators, int names ) 
{
  CMPIStatus      st = {CMPI_RC_OK,NULL};
  CMPIString     *clsname, *namesp;
  CMPIObjectPath *co;
  CMPIInstance   *ci;
  CMPIData        iddata;
  char            metricname[500];
  char            metricidc[100];
  int             metricid;
  char          **metricnames;
  char          **resources;
  char          **systems;
  int            *metricids;
  int             midnum, ridnum, i;
  COMMHEAP        ch;
  MetricResourceId *rid;

  fprintf(stderr,"--- Linux_MetricDefForME associatorHelper()\n");
    
  clsname=CMGetClassName(cop,NULL);
  namesp=CMGetNameSpace(cop,NULL);
  /*
   * Check if the object path belongs to a supported class
   */ 
  if (CMClassPathIsA(_broker,cop,"CIM_BaseMetricDefinition",NULL)) {

    iddata = CMGetKey(cop,"Id",NULL);
    /* Metric Definition - we search for Metric Values for this Id */
    if (iddata.value.string &&
	parseMetricDefId(CMGetCharPtr(iddata.value.string),
			 metricname,&metricid) == 0) {
      sprintf(metricidc,"%i",metricid);
      if (checkRepositoryConnection()) {
	ch=ch_init();
	ridnum = rreposresource_list(metricidc,&rid,ch);
	for (i=0;i<ridnum;i++) {
	  co = makeResourcePath(_broker,ctx,CMGetCharPtr(namesp),
				metricname,metricid,
				rid[i].mrid_resource,
				rid[i].mrid_system);
	  if (co) {
	    computeResourceNamespace(co,cop,rid[i].mrid_system);
	    if (names && associators) {
	      CMReturnObjectPath(rslt,co);
	    } else if (!names && associators) {
	      ci = CBGetInstance(_broker,ctx,co,NULL,NULL);
	      // this can fail if the instance is not served by our CIM Server
	      if (ci) {
		CMReturnInstance(rslt,ci);	      
	      }
	    } else if (names) {
	      CMReturnObjectPath(rslt,_makeRefPath(co, cop));
	    } else {
	      CMReturnInstance(rslt,_makeRefInstance(co, cop));
	    }
	  }
	}
	ch_release(ch);
      }
    }
  } else {
    /* get all metric values for resource class */
    midnum=getMetricIdsForResourceClass(_broker,ctx,cop,
					&metricnames,
					&metricids,
					&resources,
					&systems);
    for(i=0; i<midnum; i++) {
      co = makeMetricDefPath( _broker, ctx,
			      metricnames[i],
			      metricids[i],
			      CMGetCharPtr(namesp),
			      &st );
      if (names && associators) {
	CMReturnObjectPath(rslt,co);
      } else if (!names && associators) {
	ci = makeMetricDefInst( _broker, ctx,
				metricnames[i], 
				metricids[i],
				CMGetCharPtr(namesp),
				&st );
	CMReturnInstance(rslt,ci);	      
      } else if (names) {
	CMReturnObjectPath(rslt,_makeRefPath(cop, co));
      } else {
	CMReturnInstance(rslt,_makeRefInstance(cop, co));
      }
    }
    releaseMetricIds(metricnames,metricids,resources,systems);
  }
  CMReturnDone(rslt);
  
  return st;
}

/* ------------------------------------------------------------------ *
 * Instance MI Cleanup 
 * ------------------------------------------------------------------ */

CMPIStatus OSBase_MetricDefForMEProviderCleanup( CMPIInstanceMI * mi, 
				 CMPIContext * ctx) 
{
  releaseMetricDefClasses();
  CMReturn(CMPI_RC_OK);
}

/* ------------------------------------------------------------------ *
 * Instance MI Functions
 * ------------------------------------------------------------------ */


CMPIStatus OSBase_MetricDefForMEProviderEnumInstanceNames( CMPIInstanceMI * mi, 
					   CMPIContext * ctx, 
					   CMPIResult * rslt, 
					   CMPIObjectPath * ref) 
{
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus OSBase_MetricDefForMEProviderEnumInstances( CMPIInstanceMI * mi, 
				       CMPIContext * ctx, 
				       CMPIResult * rslt, 
				       CMPIObjectPath * ref, 
				       char ** properties) 
{
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}


CMPIStatus OSBase_MetricDefForMEProviderGetInstance( CMPIInstanceMI * mi, 
				     CMPIContext * ctx, 
				     CMPIResult * rslt, 
				     CMPIObjectPath * cop, 
				     char ** properties) 
{
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus OSBase_MetricDefForMEProviderCreateInstance( CMPIInstanceMI * mi, 
					CMPIContext * ctx, 
					CMPIResult * rslt, 
					CMPIObjectPath * cop, 
					CMPIInstance * ci) 
{
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus OSBase_MetricDefForMEProviderSetInstance( CMPIInstanceMI * mi, 
				     CMPIContext * ctx, 
				     CMPIResult * rslt, 
				     CMPIObjectPath * cop,
				     CMPIInstance * ci, 
				     char **properties) 
{
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus OSBase_MetricDefForMEProviderDeleteInstance( CMPIInstanceMI * mi, 
					CMPIContext * ctx, 
					CMPIResult * rslt, 
					CMPIObjectPath * cop) 
{ 
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus OSBase_MetricDefForMEProviderExecQuery( CMPIInstanceMI * mi, 
				   CMPIContext * ctx, 
				   CMPIResult * rslt, 
				   CMPIObjectPath * cop, 
				   char * lang, 
				   char * query) 
{
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

/* ------------------------------------------------------------------ *
 * Association MI Cleanup 
 * ------------------------------------------------------------------ */

CMPIStatus OSBase_MetricDefForMEProviderAssociationCleanup( CMPIAssociationMI * mi,
					      CMPIContext * ctx) 
{
  releaseMetricDefClasses();
  CMReturn(CMPI_RC_OK);
}

/* ------------------------------------------------------------------ *
 * Association MI Functions
 * ------------------------------------------------------------------ */

CMPIStatus OSBase_MetricDefForMEProviderAssociators( CMPIAssociationMI * mi,
				       CMPIContext * ctx,
				       CMPIResult * rslt,
				       CMPIObjectPath * cop,
				       const char * assocClass,
				       const char * resultClass,
				       const char * role,
				       const char * resultRole,
				       char ** propertyList ) 
{
  return associatorHelper(rslt,ctx,cop,1,0);
}

CMPIStatus OSBase_MetricDefForMEProviderAssociatorNames( CMPIAssociationMI * mi,
					   CMPIContext * ctx,
					   CMPIResult * rslt,
					   CMPIObjectPath * cop,
					   const char * assocClass,
					   const char * resultClass,
					   const char * role,
					   const char * resultRole ) 
{
  return associatorHelper(rslt,ctx,cop,1,1);
}

CMPIStatus OSBase_MetricDefForMEProviderReferences( CMPIAssociationMI * mi,
				      CMPIContext * ctx,
				      CMPIResult * rslt,
				      CMPIObjectPath * cop,
				      const char * assocClass,
				      const char * role,
				      char ** propertyList ) 
{
  return associatorHelper(rslt,ctx,cop,0,0);
}


CMPIStatus OSBase_MetricDefForMEProviderReferenceNames( CMPIAssociationMI * mi,
					  CMPIContext * ctx,
					  CMPIResult * rslt,
					  CMPIObjectPath * cop,
					  const char * assocClass,
					  const char * role) 
{
  return associatorHelper(rslt,ctx,cop,0,1);
}

/* ------------------------------------------------------------------ *
 * Instance MI Factory
 * 
 * NOTE: This is an example using the convenience macros. This is OK 
 *       as long as the MI has no special requirements, i.e. to store
 *       data between calls.
 * ------------------------------------------------------------------ */

CMInstanceMIStub( OSBase_MetricDefForMEProvider,
		  OSBase_MetricDefForMEProvider,
		  _broker,
		  CMNoHook);

/* ------------------------------------------------------------------ *
 * Association MI Factory
 * ------------------------------------------------------------------ */

CMAssociationMIStub( OSBase_MetricDefForMEProvider,
		     OSBase_MetricDefForMEProvider,
		     _broker,
		     CMNoHook);
