/*
 * $Id: OSBase_MetricForMEProvider.c,v 1.7 2007/07/10 13:23:33 mihajlov Exp $
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
 * Author:       Viktor Mihajlovski <mihajlov@de.ibm.com>
 *
 * Interface Type : Common Manageability Programming Interface ( CMPI )
 *
 * Description: Association Provider for CIM_MetricForMe
 * 
 */


#include <cmpidt.h>
#include <cmpift.h>
#include <cmpimacs.h>

#include <stdio.h>

#include <rrepos.h>
#include "OSBase_MetricUtil.h"

#define LOCALCLASSNAME "Linux_MetricForME"

static CMPIBroker * _broker;

/* ------------------------------------------------------------------ *
 * Simplified Utility Functions
 * no support for assocClass, resultClass, etc.
 * ------------------------------------------------------------------ */

static CMPIInstance * _makeRefInstance(CMPIObjectPath *defp,
				       CMPIObjectPath *valp)
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

static CMPIObjectPath * _makeRefPath(CMPIObjectPath *defp,
				     CMPIObjectPath *valp)
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
  char            resource[500];
  char            systemid[500];
  time_t          timestamp;
  int             metricid;
  char          **metricnames;
  char          **resources;
  char          **systems;
  int            *metricids;
  int             midnum, i, j;
  COMMHEAP        ch;
  ValueRequest    vr;

  fprintf(stderr,"--- associatorHelper()\n");
    
  clsname=CMGetClassName(cop,NULL);
  namesp=CMGetNameSpace(cop,NULL);
  /*
   * Check if the object path belongs to a supported class
   */ 
  if (CMClassPathIsA(_broker,cop,"CIM_BaseMetricValue",NULL)) {
    iddata = CMGetKey(cop,"InstanceId",NULL);
    if (iddata.value.string &&
	parseMetricValueId(CMGetCharPtr(iddata.value.string),
			   metricname,&metricid,resource,systemid,&timestamp) == 0) {
      co = makeResourcePath(_broker,ctx,CMGetCharPtr(namesp),
			    metricname,metricid,resource,systemid);
      if (co) {
	computeResourceNamespace(co,cop,systemid);
	if (names && associators) {
	  CMReturnObjectPath(rslt,co);
	} else if (!names && associators) {
	  ci = CBGetInstance(_broker,ctx,co,NULL,NULL);
	  /* this can fail if the instance is not served by our CIM Server */
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
  } else {
    /* get all metric values for resource class */
    midnum=getMetricIdsForResourceClass(_broker,ctx,cop,
					&metricnames,
					&metricids,
					&resources,
					&systems);
    if (checkRepositoryConnection()) {
      ch=ch_init();
      for(i=0; i<midnum; i++) {
	/* for all metric ids retrieve data for given resource */
	vr.vsId=metricids[i];
	vr.vsResource=resources[i];
	vr.vsSystemId=systems[i];
	vr.vsFrom=vr.vsTo=0;
	vr.vsNumValues=1; /* restrict to one/latest value per resource */
	if (rrepos_get(&vr,ch)==0) {
	  for (j=0; j < vr.vsNumValues; j++) {
	    co = makeMetricValuePath( _broker, ctx,
				      metricnames[i], metricids[i],
				      &vr.vsValues[j], 
				      cop, &st );
	    if (co==NULL) {
	      continue;
	    }
	    if (names && associators) {
	      CMReturnObjectPath(rslt,co);
	    } else if (!names && associators) {
	      ci = makeMetricValueInst( _broker, ctx, metricnames[i], 
					metricids[i],
					&vr.vsValues[j],
					vr.vsDataType,
					cop, &st );
	      if (ci) {
		CMReturnInstance(rslt,ci);	      
	      }
	    } else if (names) {
	      CMReturnObjectPath(rslt,_makeRefPath(cop, co));
	    } else {
	      CMReturnInstance(rslt,_makeRefInstance(cop, co));
	    }
	  }
	}
      }
      ch_release(ch);
    }
    releaseMetricIds(metricnames,metricids,resources,systems);
  }
  CMReturnDone(rslt);
  
  return st;
}

/* ------------------------------------------------------------------ *
 * Instance MI Cleanup 
 * ------------------------------------------------------------------ */

CMPIStatus OSBase_MetricForMEProviderCleanup( CMPIInstanceMI * mi, 
				 CMPIContext * ctx) 
{
  releaseMetricDefClasses();
  CMReturn(CMPI_RC_OK);
}

/* ------------------------------------------------------------------ *
 * Instance MI Functions
 * ------------------------------------------------------------------ */


CMPIStatus OSBase_MetricForMEProviderEnumInstanceNames( CMPIInstanceMI * mi, 
					   CMPIContext * ctx, 
					   CMPIResult * rslt, 
					   CMPIObjectPath * ref) 
{
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus OSBase_MetricForMEProviderEnumInstances( CMPIInstanceMI * mi, 
				       CMPIContext * ctx, 
				       CMPIResult * rslt, 
				       CMPIObjectPath * ref, 
				       char ** properties) 
{
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}


CMPIStatus OSBase_MetricForMEProviderGetInstance( CMPIInstanceMI * mi, 
				     CMPIContext * ctx, 
				     CMPIResult * rslt, 
				     CMPIObjectPath * cop, 
				     char ** properties) 
{
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus OSBase_MetricForMEProviderCreateInstance( CMPIInstanceMI * mi, 
					CMPIContext * ctx, 
					CMPIResult * rslt, 
					CMPIObjectPath * cop, 
					CMPIInstance * ci) 
{
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus OSBase_MetricForMEProviderSetInstance( CMPIInstanceMI * mi, 
				     CMPIContext * ctx, 
				     CMPIResult * rslt, 
				     CMPIObjectPath * cop,
				     CMPIInstance * ci, 
				     char **properties) 
{
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus OSBase_MetricForMEProviderDeleteInstance( CMPIInstanceMI * mi, 
					CMPIContext * ctx, 
					CMPIResult * rslt, 
					CMPIObjectPath * cop) 
{ 
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus OSBase_MetricForMEProviderExecQuery( CMPIInstanceMI * mi, 
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

CMPIStatus OSBase_MetricForMEProviderAssociationCleanup( CMPIAssociationMI * mi,
					      CMPIContext * ctx) 
{
  releaseMetricDefClasses();
  CMReturn(CMPI_RC_OK);
}

/* ------------------------------------------------------------------ *
 * Association MI Functions
 * ------------------------------------------------------------------ */

CMPIStatus OSBase_MetricForMEProviderAssociators( CMPIAssociationMI * mi,
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

CMPIStatus OSBase_MetricForMEProviderAssociatorNames( CMPIAssociationMI * mi,
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

CMPIStatus OSBase_MetricForMEProviderReferences( CMPIAssociationMI * mi,
				      CMPIContext * ctx,
				      CMPIResult * rslt,
				      CMPIObjectPath * cop,
				      const char * assocClass,
				      const char * role,
				      char ** propertyList ) 
{
  return associatorHelper(rslt,ctx,cop,0,0);
}


CMPIStatus OSBase_MetricForMEProviderReferenceNames( CMPIAssociationMI * mi,
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

CMInstanceMIStub( OSBase_MetricForMEProvider,
		  OSBase_MetricForMEProvider,
		  _broker,
		  CMNoHook);

/* ------------------------------------------------------------------ *
 * Association MI Factory
 * ------------------------------------------------------------------ */

CMAssociationMIStub( OSBase_MetricForMEProvider,
		     OSBase_MetricForMEProvider,
		     _broker,
		     CMNoHook);
