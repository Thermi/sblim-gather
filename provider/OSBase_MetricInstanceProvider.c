/*
 * $Id: OSBase_MetricInstanceProvider.c,v 1.1 2004/09/24 12:06:30 mihajlov Exp $
 *
 * Copyright (c) 2004, International Business Machines
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
 * Description: Association Provider for CIM_MetricInstance
 * 
 */


#include <cmpidt.h>
#include <cmpift.h>
#include <cmpimacs.h>
#include <rrepos.h>
#include "OSBase_MetricUtil.h"

#define LOCALCLASSNAME "Linux_MetricInstance"

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
  char            defclass[500];
  char            metricname[500];
  int             metricid;
  int             i;
  ValueRequest    vr;
  COMMHEAP        ch;

  fprintf(stderr,"--- associatorHelper()\n");
    
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
      if (checkRepositoryConnection()) {
	ch=ch_init();
	vr.vsId=metricid;
	vr.vsResource=NULL;
	vr.vsFrom=vr.vsTo=0;
	rrepos_get(&vr,ch);
	for (i=0; i < vr.vsNumValues; i++) {
	  co = makeMetricValuePath( _broker, ctx,
				    metricname, metricid,
				    &vr.vsValues[i], 
				    cop, &st );
	  if (names && associators) {
	    CMReturnObjectPath(rslt,co);
	  } else if (!names && associators) {
	    ci = makeMetricValueInst( _broker, ctx, metricname, metricid,
				      &vr.vsValues[i],
				      vr.vsDataType,
				      cop, &st );
	    CMReturnInstance(rslt,ci);	      
	  } else if (names) {
	    CMReturnObjectPath(rslt,_makeRefPath(cop, co));
	  } else {
	    CMReturnInstance(rslt,_makeRefInstance(cop, co));
	  }
	}
	ch_release(ch);
      }
    }
  } else if (CMClassPathIsA(_broker,cop,"CIM_BaseMetricValue",NULL)) {
    iddata = CMGetKey(cop,"MetricDefinitionId",NULL);
    /* Must be a metric value */
    if (iddata.value.string &&
	parseMetricDefId(CMGetCharPtr(iddata.value.string),
			 metricname,&metricid) == 0) {
      if (metricDefClassName(_broker,ctx,
			     CMGetCharPtr(namesp),
			     defclass,metricname,metricid) == 0) {
	co = makeMetricDefPath(_broker,ctx,
			       metricname,metricid,CMGetCharPtr(namesp),
			       &st); 
	if (names && associators) {
	  CMReturnObjectPath(rslt,co);
	} else if (!names && associators) {
	  ci = makeMetricDefInst( _broker, ctx, metricname, metricid,
				  CMGetCharPtr(namesp), &st );
	  CMReturnInstance(rslt,ci);	      
	} else if (names) {
	  CMReturnObjectPath(rslt,_makeRefPath(co, cop));
	} else {
	  CMReturnInstance(rslt,_makeRefInstance(co, cop));
	}
      }
      
    }
  } else {
    fprintf(stderr,"--- unsupported class %s\n",CMGetCharPtr(clsname));
  }
  CMReturnDone(rslt);
  
  return st;
}

/* ------------------------------------------------------------------ *
 * Instance MI Cleanup 
 * ------------------------------------------------------------------ */

CMPIStatus OSBase_MetricInstanceProviderCleanup( CMPIInstanceMI * mi, 
				 CMPIContext * ctx) 
{
  releaseMetricDefClasses();
  CMReturn(CMPI_RC_OK);
}

/* ------------------------------------------------------------------ *
 * Instance MI Functions
 * ------------------------------------------------------------------ */


CMPIStatus OSBase_MetricInstanceProviderEnumInstanceNames( CMPIInstanceMI * mi, 
					   CMPIContext * ctx, 
					   CMPIResult * rslt, 
					   CMPIObjectPath * ref) 
{
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus OSBase_MetricInstanceProviderEnumInstances( CMPIInstanceMI * mi, 
				       CMPIContext * ctx, 
				       CMPIResult * rslt, 
				       CMPIObjectPath * ref, 
				       char ** properties) 
{
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}


CMPIStatus OSBase_MetricInstanceProviderGetInstance( CMPIInstanceMI * mi, 
				     CMPIContext * ctx, 
				     CMPIResult * rslt, 
				     CMPIObjectPath * cop, 
				     char ** properties) 
{
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus OSBase_MetricInstanceProviderCreateInstance( CMPIInstanceMI * mi, 
					CMPIContext * ctx, 
					CMPIResult * rslt, 
					CMPIObjectPath * cop, 
					CMPIInstance * ci) 
{
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus OSBase_MetricInstanceProviderSetInstance( CMPIInstanceMI * mi, 
				     CMPIContext * ctx, 
				     CMPIResult * rslt, 
				     CMPIObjectPath * cop,
				     CMPIInstance * ci, 
				     char **properties) 
{
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus OSBase_MetricInstanceProviderDeleteInstance( CMPIInstanceMI * mi, 
					CMPIContext * ctx, 
					CMPIResult * rslt, 
					CMPIObjectPath * cop) 
{ 
  CMReturn( CMPI_RC_ERR_NOT_SUPPORTED );
}

CMPIStatus OSBase_MetricInstanceProviderExecQuery( CMPIInstanceMI * mi, 
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

CMPIStatus OSBase_MetricInstanceProviderAssociationCleanup( CMPIAssociationMI * mi,
					      CMPIContext * ctx) 
{
  releaseMetricDefClasses();
  CMReturn(CMPI_RC_OK);
}

/* ------------------------------------------------------------------ *
 * Association MI Functions
 * ------------------------------------------------------------------ */

CMPIStatus OSBase_MetricInstanceProviderAssociators( CMPIAssociationMI * mi,
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

CMPIStatus OSBase_MetricInstanceProviderAssociatorNames( CMPIAssociationMI * mi,
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

CMPIStatus OSBase_MetricInstanceProviderReferences( CMPIAssociationMI * mi,
				      CMPIContext * ctx,
				      CMPIResult * rslt,
				      CMPIObjectPath * cop,
				      const char * assocClass,
				      const char * role,
				      char ** propertyList ) 
{
  return associatorHelper(rslt,ctx,cop,0,0);
}


CMPIStatus OSBase_MetricInstanceProviderReferenceNames( CMPIAssociationMI * mi,
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

CMInstanceMIStub( OSBase_MetricInstanceProvider,
		  OSBase_MetricInstanceProvider,
		  _broker,
		  CMNoHook);

/* ------------------------------------------------------------------ *
 * Association MI Factory
 * ------------------------------------------------------------------ */

CMAssociationMIStub( OSBase_MetricInstanceProvider,
		     OSBase_MetricInstanceProvider,
		     _broker,
		     CMNoHook);
