/*
 * $Id: OSBase_MetricRepositoryServiceProvider.c,v 1.1 2004/08/05 11:35:42 mihajlov Exp $
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
 * Contributors:
 *
 * Interface Type : Common Manageability Programming Interface ( CMPI )
 *
 * Description: Provider for the SBLIM RepositoryService.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <metric.h>
#include <rrepos.h>

#include "cmpidt.h"
#include "cmpift.h"
#include "cmpimacs.h"

#include <OSBase_Common.h> 
#include <cmpiOSBase_Common.h> 

#define _debug 1

CMPIBroker * _broker;

/* ---------------------------------------------------------------------------*/
/* private declarations                                                       */

static char * _ClassName = "Linux_MetricRepositoryService";
static char * _Name = "reposd";
static char * _FILENAME = "OSBase_MetricRepositoryServiceProvider.c";

static char  _false=0;


/* ---------------------------------------------------------------------------*/


/* ---------------------------------------------------------------------------*/
/*                      Instance Provider Interface                           */
/* ---------------------------------------------------------------------------*/


CMPIStatus OSBase_MetricRepositoryServiceProviderCleanup( CMPIInstanceMI * mi, 
           CMPIContext * ctx) { 
  if( _debug )
    fprintf( stderr, "--- %s : %s CMPI Cleanup()\n", _FILENAME, _ClassName );
  CMReturn(CMPI_RC_OK);
}

CMPIStatus OSBase_MetricRepositoryServiceProviderEnumInstanceNames( CMPIInstanceMI * mi, 
           CMPIContext * ctx, 
           CMPIResult * rslt, 
           CMPIObjectPath * ref) { 
  CMPIObjectPath * op = NULL;
  CMPIStatus       rc = {CMPI_RC_OK, NULL};
  
  if( _debug )
    fprintf( stderr, "--- %s : %s CMPI EnumInstanceNames()\n", _FILENAME, _ClassName );

  /* we assume there is exactly one of us */
  op=CMNewObjectPath(_broker,CMGetCharPtr(CMGetNameSpace(ref,NULL)),_ClassName,
		     NULL);
  if (op) {
    CMAddKey(op,"CreationClassName",_ClassName,CMPI_chars);
    CMAddKey(op,"Name",_Name,CMPI_chars);
    CMAddKey(op,"SystemCreationClassName",CSCreationClassName,CMPI_chars);
    CMAddKey(op,"SystemName",get_system_name(),CMPI_chars);
    CMReturnObjectPath(rslt,op);
  } else {
    CMSetStatusWithChars( _broker, &rc, 
			  CMPI_RC_ERR_FAILED, "Could not build object path" ); 
  }
  CMReturnDone( rslt );
  return rc;
}

CMPIStatus OSBase_MetricRepositoryServiceProviderEnumInstances( CMPIInstanceMI * mi, 
           CMPIContext * ctx, 
           CMPIResult * rslt, 
           CMPIObjectPath * ref, 
           char ** properties) { 
  CMPIObjectPath * op = NULL;
  CMPIInstance * ci = NULL;
  CMPIStatus     rc = {CMPI_RC_OK, NULL};
  RepositoryStatus   rs;
  short          ival;
  short          nump;
  short          numm;
  char           boolval;
    
  if( _debug )
    fprintf( stderr, "--- %s : %s CMPI EnumInstances()\n", _FILENAME, _ClassName );

  op=CMNewObjectPath(_broker,CMGetCharPtr(CMGetNameSpace(ref,NULL)),_ClassName,
		     NULL);
  if (op) {
    ci=CMNewInstance(_broker,op,NULL);
  } 
  if (ci) {
    CMSetProperty(ci,"CreationClassName",_ClassName,CMPI_chars);
    CMSetProperty(ci,"Name",_Name,CMPI_chars);
    CMSetProperty(ci,"SystemCreationClassName",CSCreationClassName,CMPI_chars);
    CMSetProperty(ci,"SystemName",get_system_name(),CMPI_chars);
    if (rrepos_status(&rs)) {
      CMSetProperty(ci,"Started",&_false,CMPI_boolean);
      ival=0; /* unknown */
      CMSetProperty(ci,"EnabledState",&ival,CMPI_uint16);
    } else {
      boolval=rs.rsInitialized;
      CMSetProperty(ci,"Started",&boolval,CMPI_boolean);
      if (rs.rsInitialized)
	ival=2;
      else
	ival=0;
      CMSetProperty(ci,"EnabledState",&ival,CMPI_uint16);
      nump = rs.rsNumPlugins; 
      CMSetProperty(ci,"NumberOfPlugins",&nump,CMPI_uint16);
      numm = rs.rsNumMetrics;
      CMSetProperty(ci,"NumberOfMetrics",&numm,CMPI_uint16);
    }
    CMReturnInstance(rslt,ci);
  } else {
    CMSetStatusWithChars( _broker, &rc, 
			  CMPI_RC_ERR_FAILED, "Could not build object path" ); 
  }
  CMReturnDone( rslt );
  return rc;
}

CMPIStatus OSBase_MetricRepositoryServiceProviderGetInstance( CMPIInstanceMI * mi, 
           CMPIContext * ctx, 
           CMPIResult * rslt, 
           CMPIObjectPath * ref, 
           char ** properties) {
  CMPIObjectPath * op = NULL;
  CMPIInstance * ci = NULL;
  CMPIStatus     rc = {CMPI_RC_OK, NULL};
  RepositoryStatus   rs;
  short          ival;
  short          nump;
  short          numm;
  char           boolval;

  if( _debug )
    fprintf( stderr, "--- %s : %s CMPI GetInstance()\n", _FILENAME, _ClassName );
  op=CMNewObjectPath(_broker,CMGetCharPtr(CMGetNameSpace(ref,NULL)),_ClassName,
		     NULL);
  if (op) {
    ci=CMNewInstance(_broker,op,NULL);
  } 
  if (ci) {
    CMSetProperty(ci,"CreationClassName",_ClassName,CMPI_chars);
    CMSetProperty(ci,"Name",_Name,CMPI_chars);
    CMSetProperty(ci,"SystemCreationClassName",CSCreationClassName,CMPI_chars);
    CMSetProperty(ci,"SystemName",get_system_name(),CMPI_chars);
    if (rrepos_status(&rs)) {
      CMSetProperty(ci,"Started",&_false,CMPI_boolean);
      ival=0; /* unknown */
      CMSetProperty(ci,"EnabledState",&ival,CMPI_uint16);
    } else {
      boolval=rs.rsInitialized;
      CMSetProperty(ci,"Started",&boolval,CMPI_boolean);
      if (rs.rsInitialized)
	ival=2;
      else
	ival=0;
      CMSetProperty(ci,"EnabledState",&ival,CMPI_uint16);
      nump = rs.rsNumPlugins;
      CMSetProperty(ci,"NumberOfPlugins",&numm,CMPI_uint16);
      numm = rs.rsNumMetrics;
      CMSetProperty(ci,"NumberOfMetrics",&numm,CMPI_uint16);
    }
    CMReturnInstance(rslt,ci);
  } else {
    CMSetStatusWithChars( _broker, &rc, 
			  CMPI_RC_ERR_FAILED, "RepositoryService Service not active" ); 
  }
  CMReturnDone(rslt);
  return rc;
}

CMPIStatus OSBase_MetricRepositoryServiceProviderCreateInstance( CMPIInstanceMI * mi, 
           CMPIContext * ctx, 
           CMPIResult * rslt, 
           CMPIObjectPath * cop, 
           CMPIInstance * ci) {
  CMPIStatus rc = {CMPI_RC_OK, NULL};

  if( _debug )
    fprintf( stderr, "--- %s : %s CMPI CreateInstance()\n", _FILENAME, _ClassName );

  CMSetStatusWithChars( _broker, &rc, 
			CMPI_RC_ERR_NOT_SUPPORTED, "CIM_ERR_NOT_SUPPORTED" ); 
  return rc;
}

CMPIStatus OSBase_MetricRepositoryServiceProviderSetInstance( CMPIInstanceMI * mi, 
           CMPIContext * ctx, 
           CMPIResult * rslt, 
           CMPIObjectPath * cop,
           CMPIInstance * ci, 
           char **properties) {
  CMPIStatus rc = {CMPI_RC_OK, NULL};

  if( _debug )
    fprintf( stderr, "--- %s : %s CMPI SetInstance()\n", _FILENAME, _ClassName );

  CMSetStatusWithChars( _broker, &rc, 
			CMPI_RC_ERR_NOT_SUPPORTED, "CIM_ERR_NOT_SUPPORTED" ); 
  return rc;
}

CMPIStatus OSBase_MetricRepositoryServiceProviderDeleteInstance( CMPIInstanceMI * mi, 
           CMPIContext * ctx, 
           CMPIResult * rslt, 
           CMPIObjectPath * cop) {
  CMPIStatus rc = {CMPI_RC_OK, NULL}; 

  if( _debug )
    fprintf( stderr, "--- %s : %s CMPI DeleteInstance()\n", _FILENAME, _ClassName );

  CMSetStatusWithChars( _broker, &rc, 
			CMPI_RC_ERR_NOT_SUPPORTED, "CIM_ERR_NOT_SUPPORTED" ); 
  return rc;
}

CMPIStatus OSBase_MetricRepositoryServiceProviderExecQuery( CMPIInstanceMI * mi, 
           CMPIContext * ctx, 
           CMPIResult * rslt, 
           CMPIObjectPath * ref, 
           char * lang, 
           char * query) {
  CMPIStatus rc = {CMPI_RC_OK, NULL};

  if( _debug )
    fprintf( stderr, "--- %s : %s CMPI ExecQuery()\n", _FILENAME, _ClassName );

  CMSetStatusWithChars( _broker, &rc, 
			CMPI_RC_ERR_NOT_SUPPORTED, "CIM_ERR_NOT_SUPPORTED" ); 
  return rc;
}

/* ---------------------------------------------------------------------------*/
/*                      Method Provider Interface                             */
/* ---------------------------------------------------------------------------*/
CMPIStatus OSBase_MetricRepositoryServiceProviderMethodCleanup( CMPIMethodMI * mi, 
					       CMPIContext * ctx) 
{
  if( _debug )
    fprintf( stderr, "--- %s : %s CMPI MethodCleanup()\n", _FILENAME, _ClassName );

  CMReturn(CMPI_RC_OK);
}

CMPIStatus OSBase_MetricRepositoryServiceProviderInvokeMethod( CMPIMethodMI * mi, 
					      CMPIContext * ctx, 
					      CMPIResult * rslt,
					      CMPIObjectPath * cop,
					      char * method,
					      CMPIArgs * in,
					      CMPIArgs * out)
{
  CMPIStatus   st = {CMPI_RC_OK,NULL};
  RepositoryStatus rs;
  CMPIData     data;
  CMPIEnumeration * en;
  CMPIObjectPath  * copPlugin;
  int          stat;

  if( _debug )
    fprintf( stderr, "--- %s : %s CMPI InvokeMethod()\n", _FILENAME, _ClassName );

  if (rrepos_status(&rs)) {
    rs.rsInitialized=0;
  }

  if (strcasecmp(method,"startservice")==0) {
    if( _debug )
      fprintf( stderr, "--- invoking startservice()\n");
    if (!rs.rsInitialized) {
      rrepos_load(); /* attempt to start the daemon - just in case */
      if (rrepos_init()==0) {
	if( _debug )
	  fprintf( stderr, "--- attempting init\n");
	stat=0;
	/* now load plugins */
	copPlugin = CMNewObjectPath(_broker,
				    CMGetCharPtr(CMGetNameSpace(cop,NULL)),
				    "Linux_RepositoryPlugin",NULL);
	if (copPlugin) {
	  if( _debug )
	    fprintf( stderr, "--- searching plugins\n");
	  en = CBEnumInstances(_broker,ctx,copPlugin,NULL,NULL);
	  while (CMHasNext(en,NULL)) {
	    data = CMGetNext(en,NULL);
	    if (data.value.inst) {
	      data=CMGetProperty(data.value.inst,"RepositoryPluginName",NULL);
	      if (data.type == CMPI_string && data.value.string) {
		if( _debug )
		  fprintf( stderr, "--- adding plugin %s\n",
			   CMGetCharPtr(data.value.string));
		rreposplugin_add(CMGetCharPtr(data.value.string));
	      }
	    }
	  }
	}
      } else
	stat=1;
    } else {
      stat=0;
    }
    CMReturnData(rslt,&stat,CMPI_uint32);
  } else if (strcasecmp(method,"stopservice") ==0) {
    if( _debug )
      fprintf( stderr, "--- invoking stopservice()\n");
    if (rs.rsInitialized) {
      if (rrepos_terminate()==0)
	stat=0;
      else
	stat=1;
    } else {
      stat=0;
    }
    CMReturnData(rslt,&stat,CMPI_uint32);   
  } else {
    CMSetStatusWithChars( _broker, &st, 
			  CMPI_RC_ERR_NOT_SUPPORTED, "CIM_ERR_NOT_SUPPORTED" ); 
  }
  
  CMReturnDone(rslt);
  
  return st;
}

/* ---------------------------------------------------------------------------*/
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/

CMInstanceMIStub( OSBase_MetricRepositoryServiceProvider, 
                  OSBase_MetricRepositoryServiceProvider, 
                  _broker, 
                  CMNoHook);

CMMethodMIStub( OSBase_MetricRepositoryServiceProvider, 
		OSBase_MetricRepositoryServiceProvider, 
		_broker, 
		CMNoHook);

/* ---------------------------------------------------------------------------*/
/*                               Private Stuff                                */
/* ---------------------------------------------------------------------------*/


/* ---------------------------------------------------------------------------*/
/*              end of OSBase_MetricRepositoryServiceProvider                 */
/* ---------------------------------------------------------------------------*/
