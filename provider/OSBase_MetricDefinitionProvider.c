/*
 * $Id: OSBase_MetricDefinitionProvider.c,v 1.2 2004/06/09 17:19:13 mihajlov Exp $
 *
 * Copyright (c) 2003, International Business Machines
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
 * Description: Generic Provider for Metric Defintions served by
 *              the SBLIM Gatherer.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <rgather.h>
#include <metric.h>

#include "cmpidt.h"
#include "cmpift.h"
#include "cmpimacs.h"


#define _debug 0

CMPIBroker * _broker;

/* ---------------------------------------------------------------------------*/
/* private declarations                                                       */

static char * _ClassName = "CIM_BaseMetricDefinition";
static char * _FILENAME = "OSBase_MetricDefinitionProvider.c";


static int checkConnection();

static char * metricPluginName(CMPIBroker * broker, 
			       CMPIContext *context,
			       CMPIObjectPath *path);
static char * metricClassName(CMPIObjectPath *path);
static char * makeId(char * defid, const char * name, int id);
static int parseId(const char * defid,
		   char * name, int * id);

static CMPIObjectPath * _pluginPath(CMPIBroker * broker,
				    CMPIObjectPath *cop,
				    CMPIStatus *rc);
static CMPIInstance * _makeInst(CMPIBroker * broker, 
				PluginDefinition *def,
				CMPIObjectPath *cop,
				CMPIStatus * rc);
static CMPIObjectPath * _makePath(CMPIBroker * broker, 
				  PluginDefinition *def, 
				  CMPIObjectPath *cop,
				  CMPIStatus * rc);
static void returnPathes(CMPIBroker * broker, 
			  PluginDefinition *def, 
			  int defnum,
			  CMPIObjectPath *cop,
			  CMPIResult *rslt,
			  CMPIStatus * rc);
static void returnInstances(CMPIBroker * broker, 
			    PluginDefinition *def, 
			    int defnum,
			    CMPIObjectPath *cop,
			    CMPIResult *rslt,
			    CMPIStatus * rc);
/* ---------------------------------------------------------------------------*/


/* ---------------------------------------------------------------------------*/
/*                      Instance Provider Interface                           */
/* ---------------------------------------------------------------------------*/


CMPIStatus OSBase_MetricDefinitionProviderCleanup( CMPIInstanceMI * mi, 
           CMPIContext * ctx) { 
  if( _debug )
    fprintf( stderr, "--- %s : %s CMPI Cleanup()\n", _FILENAME, _ClassName );
  CMReturn(CMPI_RC_OK);
}

CMPIStatus OSBase_MetricDefinitionProviderEnumInstanceNames( CMPIInstanceMI * mi, 
           CMPIContext * ctx, 
           CMPIResult * rslt, 
           CMPIObjectPath * ref) { 
  CMPIStatus       rc = {CMPI_RC_OK, NULL};
  int              pdefnum;
  PluginDefinition *pdef;
  COMMHEAP         ch;
  char            *pname;
  CMPIEnumeration *en;
  CMPIData         plugindata;
  CMPIData         data;
  
  if( _debug )
    fprintf( stderr, "--- %s : %s CMPI EnumInstanceNames()\n", _FILENAME, _ClassName );

  if (checkConnection()) {
    ch=ch_init();
    pname = metricPluginName(_broker,ctx,ref);
    if (pname) {
      pdefnum = rmetricplugin_list(pname,&pdef,ch);
      returnPathes(_broker,pdef,pdefnum,ref,rslt,&rc);
    } else {
      en = CBEnumInstances(_broker,ctx,_pluginPath(_broker,ref,NULL),NULL,&rc);
      while (en && CMHasNext(en,NULL)) {
	data = CMGetNext(en,NULL);
	if (data.value.inst) {
	  plugindata = CMGetProperty(data.value.inst,"MetricPluginName",&rc);
	  data = CMGetProperty(data.value.inst,"MetricDefinitionClassName",&rc);
	  if (plugindata.value.string && data.value.string) {
	    pname = CMGetCharPtr(plugindata.value.string);
	    pdefnum = rmetricplugin_list(pname,&pdef,ch);
	    returnPathes(_broker,
			 pdef,
			 pdefnum,
			 CMNewObjectPath(_broker,
					 CMGetCharPtr(CMGetNameSpace(ref,NULL)),
					 CMGetCharPtr(data.value.string),NULL),
			 rslt,&rc);
	  }
	}
      }
    }
    ch_release(ch);
  } else {
    CMSetStatusWithChars( _broker, &rc, 
			  CMPI_RC_ERR_FAILED, "Gatherer Service not active" ); 
  }
  CMReturnDone( rslt );
  return rc;
}

CMPIStatus OSBase_MetricDefinitionProviderEnumInstances( CMPIInstanceMI * mi, 
           CMPIContext * ctx, 
           CMPIResult * rslt, 
           CMPIObjectPath * ref, 
           char ** properties) { 
  CMPIStatus     rc = {CMPI_RC_OK, NULL};
  int              pdefnum;
  PluginDefinition *pdef;
  COMMHEAP         ch;
  char            *pname;
  CMPIEnumeration *en;
  CMPIData         plugindata;
  CMPIData         data;

  if( _debug )
    fprintf( stderr, "--- %s : %s CMPI EnumInstances()\n", _FILENAME, _ClassName );

  if (checkConnection()) {
    ch=ch_init();
    pname = metricPluginName(_broker,ctx,ref);
    if (pname) {
      pdefnum = rmetricplugin_list(pname,&pdef,ch);
      returnInstances(_broker,pdef,pdefnum,ref,rslt,&rc);
    } else {
      en = CBEnumInstances(_broker,ctx,_pluginPath(_broker,ref,NULL),NULL,&rc);
      while (en && CMHasNext(en,NULL)) {
	data = CMGetNext(en,NULL);
	if (data.value.inst) {
	  plugindata = CMGetProperty(data.value.inst,"MetricPluginName",&rc);
	  data = CMGetProperty(data.value.inst,"MetricDefinitionClassName",&rc);
	  if (plugindata.value.string && data.value.string) {
	    pname = CMGetCharPtr(plugindata.value.string);
	    pdefnum = rmetricplugin_list(pname,&pdef,ch);
	    returnInstances(_broker,
			    pdef,
			    pdefnum,
			    CMNewObjectPath(_broker,
					    CMGetCharPtr(CMGetNameSpace(ref,NULL)),
					    CMGetCharPtr(data.value.string),NULL),
			    rslt,&rc);
	  }
	}
      }
    }
    ch_release(ch);
  } else {
    CMSetStatusWithChars( _broker, &rc, 
			  CMPI_RC_ERR_FAILED, "Gatherer Service not active" ); 
  }
  CMReturnDone( rslt );
  return rc;
}

CMPIStatus OSBase_MetricDefinitionProviderGetInstance( CMPIInstanceMI * mi, 
           CMPIContext * ctx, 
           CMPIResult * rslt, 
           CMPIObjectPath * cop, 
           char ** properties) {
  CMPIInstance * ci = NULL;
  CMPIStatus     rc = {CMPI_RC_OK, NULL};
  int              pdefnum;
  int              i;
  PluginDefinition *pdef;
  COMMHEAP         ch;
  char             mname[300];
  int              mid;
  CMPIData         idData;
  
  if( _debug )
    fprintf( stderr, "--- %s : %s CMPI GetInstance()\n", _FILENAME, _ClassName );
  
  idData = CMGetKey(cop,"Id",NULL);
  if (idData.value.string && checkConnection()) {
    ch=ch_init();
    if (parseId(CMGetCharPtr(idData.value.string),mname,&mid) == 0) {
    pdefnum = rmetricplugin_list(metricPluginName(_broker,ctx,cop),&pdef,ch);
    for (i=0; i < pdefnum; i++) {
      if( _debug )
	fprintf(stderr, "request for %s, metric name is %s\n", mname,
		pdef[i].pdName);
      if (strcmp(mname,pdef[i].pdName)==0 && mid==pdef[i].pdId) {
	ci = _makeInst( _broker, &pdef[i], cop, &rc );
	if( ci == NULL ) {
	  if( _debug ) {
	    if( rc.msg != NULL )
	      { fprintf(stderr,"rc.msg: %s\n",CMGetCharPtr(rc.msg)); }
	  }
	  break;
	}
	CMReturnInstance( rslt, ci );
	break;
      }
    }
    } else {
      CMSetStatusWithChars( _broker, &rc, 
			  CMPI_RC_ERR_INVALID_PARAMETER, "Invalid Object Path Key \"Id\"" ); 
    }
    ch_release(ch);
 } else {
    CMSetStatusWithChars( _broker, &rc, 
			  CMPI_RC_ERR_FAILED, "Gatherer Service not active" ); 
  }
  CMReturnDone(rslt);
  return rc;
}

CMPIStatus OSBase_MetricDefinitionProviderCreateInstance( CMPIInstanceMI * mi, 
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

CMPIStatus OSBase_MetricDefinitionProviderSetInstance( CMPIInstanceMI * mi, 
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

CMPIStatus OSBase_MetricDefinitionProviderDeleteInstance( CMPIInstanceMI * mi, 
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

CMPIStatus OSBase_MetricDefinitionProviderExecQuery( CMPIInstanceMI * mi, 
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
/*                              Provider Factory                              */
/* ---------------------------------------------------------------------------*/

CMInstanceMIStub( OSBase_MetricDefinitionProvider, 
                  OSBase_MetricDefinitionProvider, 
                  _broker, 
                  CMNoHook);

/* ---------------------------------------------------------------------------*/
/*                               Private Stuff                                */
/* ---------------------------------------------------------------------------*/

static int checkConnection()
{
  GatherStatus stat;
  if (rgather_status(&stat)==0 && stat.gsInitialized)
    return 1;
  else
    return 0;
}

static void returnPathes(CMPIBroker * broker, 
			  PluginDefinition *def, 
			  int defnum,
			  CMPIObjectPath *cop,
			  CMPIResult *rslt,
			  CMPIStatus * rc)
{
  int             i;
  CMPIObjectPath *op;
  
  for (i=0; i < defnum; i++) {
    op = _makePath( _broker, &def[i], cop, rc );
    if( op == NULL ) {
      if( _debug ) {
	if( rc && rc->msg != NULL )
	  { fprintf(stderr,"rc.msg: %s\n",CMGetCharPtr(rc->msg)); }
      }
      break;
    }      
    CMReturnObjectPath( rslt, op );
  }
}

static void returnInstances(CMPIBroker * broker, 
			    PluginDefinition *def, 
			    int defnum,
			    CMPIObjectPath *cop,
			    CMPIResult *rslt,
			    CMPIStatus * rc)
{
  int           i;
  CMPIInstance *inst;
  
  for (i=0; i < defnum; i++) {
    inst = _makeInst( _broker, &def[i], cop, rc );
    if( inst == NULL ) {
      if( _debug ) {
	if( rc && rc->msg != NULL )
	  { fprintf(stderr,"rc.msg: %s\n",CMGetCharPtr(rc->msg)); }
      }
      break;
    }      
    CMReturnInstance( rslt, inst );
  }
}

static char * metricPluginName(CMPIBroker *broker, CMPIContext *context,
			       CMPIObjectPath *path)
{
  /* locate plugin instance for given class */
  CMPIStatus rc;
  CMPIObjectPath *pluginpath;
  CMPIInstance   *plugininst;
  CMPIData        plugindata;
  char * clsname = metricClassName(path);
  char * pluginname = NULL;

  if (clsname) {
    if( _debug )
      fprintf(stderr,"::: requesting plugin for %s\n",clsname);
    pluginpath = _pluginPath(_broker,path,NULL);
    if (pluginpath) {
      CMAddKey(pluginpath,"MetricDefinitionClassName",clsname,CMPI_chars);
      if( _debug )
	fprintf(stderr,"::: getting instance  for %s (%s)\n",clsname,
		CMGetCharPtr(CDToString(broker,pluginpath,NULL)));
      plugininst = CBGetInstance(broker,context,pluginpath,NULL,&rc);
      if (plugininst) {
	if( _debug )
	  fprintf(stderr,"::: got instance for %s\n",clsname);
	plugindata = CMGetProperty(plugininst,"MetricPluginName",&rc);
	pluginname = plugindata.value.string ? 
	  CMGetCharPtr(plugindata.value.string) : NULL;
      }
    }
  }
  if( _debug )
    fprintf(stderr,"::: plugin name is %s\n",pluginname);
  return pluginname;
}

static char * metricClassName(CMPIObjectPath *path)
{
  /* return class name or NULL if CIM_ (base) class */
  CMPIStatus rc;
  CMPIString * clsname = CMGetClassName(path,&rc);
  char * c_clsname;
  if (clsname) c_clsname=CMGetCharPtr(clsname);
  if (c_clsname && strncasecmp(c_clsname,"cim_",strlen("cim_")))
    return c_clsname;
  else
    return NULL;
}

static int typetable[] = {
  -1,
  MD_BOOL,
  MD_CHAR16,
  -1,
  MD_FLOAT32,
  MD_FLOAT64,
  MD_SINT16,
  MD_SINT32,
  MD_SINT64,
  MD_SINT8,
  MD_STRING,
  MD_UINT16,
  MD_UINT32,
  MD_UINT64,
  MD_UINT8,
};

static CMPIObjectPath * _pluginPath(CMPIBroker * broker,
				    CMPIObjectPath *cop,
				    CMPIStatus *rc)
{   
  return CMNewObjectPath(broker,CMGetCharPtr(CMGetNameSpace(cop,NULL)),
			 "Linux_MetricPlugin",rc);
}

static CMPIInstance * _makeInst(CMPIBroker * broker, PluginDefinition *def, 
				CMPIObjectPath *cop,
				CMPIStatus * rc)
{
  CMPIObjectPath * co;
  CMPIInstance   * ci = NULL;
  char             idbuf[300];
  short            dt;

  fprintf(stderr,"::: make inst for cls %s\n",metricClassName(cop));
  co = CMNewObjectPath(broker,NULL,metricClassName(cop),rc);
  if (co) {
    CMSetNameSpaceFromObjectPath(co,cop);
    ci = CMNewInstance(broker,co,rc);
    if (ci) {
      fprintf(stderr,"::: make inst id=%d %s\n",def->pdId,def->pdName);
      CMSetProperty(ci,"Id",makeId(idbuf,def->pdName,def->pdId),CMPI_chars);
      CMSetProperty(ci,"Name",def->pdName,CMPI_chars);
      for (dt=0;dt<sizeof(typetable)/sizeof(int);dt++) {
	if (def->pdDataType==typetable[dt])
	  break;
      }
      if (dt<sizeof(typetable)/sizeof(int))
	CMSetProperty(ci,"DataType",&dt,CMPI_uint16);
    }
  }
  return ci;
}

static CMPIObjectPath * _makePath(CMPIBroker * broker, PluginDefinition *def, 
				  CMPIObjectPath *cop,
				  CMPIStatus * rc)
{
  CMPIObjectPath * co;
  char             idbuf[300];

  if (_debug)
    fprintf(stderr,"::: make path for cls %s\n",metricClassName(cop));
  co = CMNewObjectPath(broker,NULL,metricClassName(cop),rc);
  if (co) {
    CMSetNameSpaceFromObjectPath(co,cop);
    if (_debug)
      fprintf(stderr,"::: make path id=%d %s\n",def->pdId,def->pdName);
    CMAddKey(co,"Id",makeId(idbuf,def->pdName,def->pdId),CMPI_chars);
  }
  return co;
}

static char * makeId(char * defid, const char * name, int id)
{
  sprintf(defid,"%s.%d",name,id);
  return defid;
}

static int parseId(const char * defid,
		   char * name, int * id)
{
  char *nextf = strchr(defid,'.');
  if (nextf) {
  *nextf=0;
  strcpy(name,defid);
  sscanf(nextf+1,"%d",id);
  return 0;
  } else {
      return -1;
  }
}

/* ---------------------------------------------------------------------------*/
/*              end of OSBase_MetricDefinitionProvider                      */
/* ---------------------------------------------------------------------------*/

