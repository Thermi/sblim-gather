/*
 * $Id: OSBase_MetricValueProvider.c,v 1.5 2004/08/04 11:27:36 mihajlov Exp $
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
 * Description: Generic Provider for Metric Values served by
 *              the SBLIM Gatherer.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <metric.h>
#include <rrepos.h>

#include "cmpidt.h"
#include "cmpift.h"
#include "cmpimacs.h"


#define _debug 0

CMPIBroker * _broker;

/* ---------------------------------------------------------------------------*/
/* private declarations                                                       */

static char * _ClassName = "CIM_BaseMetricValue";
static char * _FILENAME = "OSBase_MetricValueProvider.c";


static int checkConnection();

static char * metricPluginName(CMPIBroker * broker, 
			       CMPIContext *context,
			       CMPIObjectPath *path);
static char * metricValueClassName(CMPIObjectPath *path);
static char * metricValueName(CMPIObjectPath *path);
static CMPIInstance * _makeInst(CMPIBroker * broker, 
				char * defname,
				int defid,
				ValueItem *val,
				unsigned   datatype,
				CMPIObjectPath *cop,
				CMPIStatus * rc);

static CMPIObjectPath * _pathFromName(CMPIBroker * broker,
				      CMPIObjectPath *cop,
				      const char *name,
				      CMPIStatus *rc);

static CMPIObjectPath * _makePath(CMPIBroker * broker,
				  char * defname,
				  int defid,
				  ValueItem *val, 
				  CMPIObjectPath *cop,
				  CMPIStatus * rc);

static char * makeInstId(char * instid, const char * name, int id, 
			 const char * resource, time_t timestamp);
static char * makeDefId(char * defid, const char * name, int id);
static int parseInstId(const char * instid,
		       char * name, int * id, char * resource,
		       time_t * timestamp);

static void returnPathes(CMPIBroker *_broker,
			 RepositoryPluginDefinition *rdef,
			 int rdefnum,
			 COMMHEAP *ch,
			 CMPIObjectPath *ref,
			 CMPIResult *rslt,
			 CMPIStatus *rc);

static void returnInstances(CMPIBroker *_broker,
			    RepositoryPluginDefinition *rdef,
			    int rdefnum,
			    COMMHEAP *ch,
			    CMPIObjectPath *ref,
			    CMPIResult *rslt,
			    CMPIStatus *rc);

/* ---------------------------------------------------------------------------*/


/* ---------------------------------------------------------------------------*/
/*                      Instance Provider Interface                           */
/* ---------------------------------------------------------------------------*/


CMPIStatus OSBase_MetricValueProviderCleanup( CMPIInstanceMI * mi, 
           CMPIContext * ctx) { 
  if( _debug )
    fprintf( stderr, "--- %s : %s CMPI Cleanup()\n", _FILENAME, _ClassName );
  CMReturn(CMPI_RC_OK);
}

CMPIStatus OSBase_MetricValueProviderEnumInstanceNames( CMPIInstanceMI * mi, 
           CMPIContext * ctx, 
           CMPIResult * rslt, 
           CMPIObjectPath * ref) { 
  CMPIStatus       rc = {CMPI_RC_OK, NULL};
  int              rdefnum;
  RepositoryPluginDefinition *rdef;
  COMMHEAP         ch;
  CMPIEnumeration  *en;
  CMPIData         data;
  char             *pname;
  
  if( _debug )
    fprintf( stderr, "--- %s : %s CMPI EnumInstanceNames()\n", _FILENAME, _ClassName );

  if (checkConnection()) {
    /* get list of metric ids */
    ch=ch_init();
    pname = metricPluginName(_broker,ctx,ref);
    if (pname) {
      rdefnum = rreposplugin_list(pname,&rdef,ch);
      returnPathes(_broker,rdef,rdefnum,ch,ref,rslt,&rc);
    } else {
      if( _debug )
	fprintf( stderr, "--- searchin value definitions\n");
      
      en = CBEnumInstanceNames(_broker,
			       ctx,
			       _pathFromName(_broker,
					     ref,
					     "Linux_MetricValueDefinition",
					     NULL),
			       &rc);
      while (en && CMHasNext(en,NULL)) {
	if( _debug )
	  fprintf( stderr, "--- processing value definition\n");	
	data = CMGetNext(en,NULL);
	if (data.value.ref) {
	  data = CMGetKey(data.value.ref,"MetricValueClassName",NULL);
	  if (data.value.string) {
	    pname = 
	      metricPluginName(_broker,ctx,
			       _pathFromName(_broker,
					     ref,
					     CMGetCharPtr(data.value.string),
					     NULL));
	    if (pname) {
	      rdefnum = rreposplugin_list(pname,&rdef,ch);
	      returnPathes(_broker,
			   rdef,
			   rdefnum,
			   ch,
			   _pathFromName(_broker,
					 ref,
					 CMGetCharPtr(data.value.string),
					 NULL),
			   rslt,
			   &rc);
	    }
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

CMPIStatus OSBase_MetricValueProviderEnumInstances( CMPIInstanceMI * mi, 
           CMPIContext * ctx, 
           CMPIResult * rslt, 
           CMPIObjectPath * ref, 
           char ** properties) { 
  CMPIStatus     rc = {CMPI_RC_OK, NULL};
  int              rdefnum;
  RepositoryPluginDefinition *rdef;
  COMMHEAP         ch;
  CMPIEnumeration  *en;
  CMPIData         data;
  char             *pname;

  if( _debug )
    fprintf( stderr, "--- %s : %s CMPI EnumInstances()\n", _FILENAME, _ClassName );

  if (checkConnection()) {
    ch=ch_init();
    pname = metricPluginName(_broker,ctx,ref);
    if (pname) {
      rdefnum = rreposplugin_list(pname,&rdef,ch);
      returnInstances(_broker,rdef,rdefnum,ch,ref,rslt,&rc);
    } else {
      if( _debug )
	fprintf( stderr, "--- searchin value definitions\n");

      en = CBEnumInstanceNames(_broker,
			       ctx,
			       _pathFromName(_broker,
					     ref,
					     "Linux_MetricValueDefinition",
					     NULL),
			       &rc);
      while (en && CMHasNext(en,NULL)) {
	if( _debug )
	  fprintf( stderr, "--- processing value definition\n");	
	data = CMGetNext(en,NULL);
	if (data.value.ref) {
	  data = CMGetKey(data.value.ref,"MetricValueClassName",NULL);
	  if (data.value.string) {
	    pname = 
	      metricPluginName(_broker,ctx,
			       _pathFromName(_broker,
					     ref,
					     CMGetCharPtr(data.value.string),
					     NULL));
	    if (pname) {
	      rdefnum = rreposplugin_list(pname,&rdef,ch);
	      returnInstances(_broker,
			      rdef,
			      rdefnum,
			      ch,
			      _pathFromName(_broker,
					    ref,
					    CMGetCharPtr(data.value.string),
					    NULL),
			      rslt,
			      &rc);
	    }
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

CMPIStatus OSBase_MetricValueProviderGetInstance( CMPIInstanceMI * mi, 
           CMPIContext * ctx, 
           CMPIResult * rslt, 
           CMPIObjectPath * cop, 
           char ** properties) {
  CMPIInstance * ci = NULL;
  CMPIStatus     rc = {CMPI_RC_OK, NULL};
  ValueRequest     vr;
  COMMHEAP         ch;
  int              vId;
  char             vName[200];
  char             vResource[200];
  time_t           vTimestamp;

  if( _debug )
    fprintf( stderr, "--- %s : %s CMPI GetInstance()\n", _FILENAME, _ClassName );
  
  if (checkConnection()) {
    ch=ch_init();
    if (parseInstId(metricValueName(cop),vName,&vId,vResource,&vTimestamp) == 0) {
  if( _debug )
    fprintf(stderr, "id criteria %s,%s, %ld\n", vName, vResource, 
	    vTimestamp);
    // get value 
    vr.vsId = vId;
    vr.vsResource = vResource;
    vr.vsFrom = vr.vsTo = vTimestamp;
    if (rrepos_get(&vr,ch)== 0) {
      if( _debug )
	fprintf( stderr, "::: got %d values for id \n", vr.vsId);
      if (vr.vsNumValues>=1) {
	ci = _makeInst( _broker, vName, vId, &vr.vsValues[0], vr.vsDataType,
			cop, &rc );    
	if( ci == NULL ) {
	  if( _debug ) {
	    if( rc.msg != NULL )
	      { fprintf(stderr,"rc.msg: %s\n",CMGetCharPtr(rc.msg)); }
	  }
	}
	CMReturnInstance( rslt, ci );
      }
    }
    } else {
	CMSetStatusWithChars( _broker, &rc, 
			      CMPI_RC_ERR_INVALID_PARAMETER, "Invalid Object Path Key \"Id\""); 
    }
    ch_release(ch);
  } else {
    CMSetStatusWithChars( _broker, &rc, 
			  CMPI_RC_ERR_FAILED, "Gatherer Service not active" ); 
  }
  CMReturnDone(rslt);
  return rc;
}

CMPIStatus OSBase_MetricValueProviderCreateInstance( CMPIInstanceMI * mi, 
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

CMPIStatus OSBase_MetricValueProviderSetInstance( CMPIInstanceMI * mi, 
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

CMPIStatus OSBase_MetricValueProviderDeleteInstance( CMPIInstanceMI * mi, 
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

CMPIStatus OSBase_MetricValueProviderExecQuery( CMPIInstanceMI * mi, 
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

CMInstanceMIStub( OSBase_MetricValueProvider, 
                  OSBase_MetricValueProvider, 
                  _broker, 
                  CMNoHook);

/* ---------------------------------------------------------------------------*/
/*                               Private Stuff                                */
/* ---------------------------------------------------------------------------*/

static int checkConnection()
{
  RepositoryStatus stat;
  if (rrepos_status(&stat)==0 && stat.rsInitialized)
    return 1;
  else
    return 0;
}

static char * metricPluginName(CMPIBroker *broker, CMPIContext *context,
			       CMPIObjectPath *path)
{
  /* locate plugin instance for given class */
  CMPIStatus rc;
  CMPIObjectPath *pluginpath;
  CMPIObjectPath *defpath;
  CMPIInstance   *plugininst;
  CMPIInstance   *definst;
  CMPIData        plugindata;
  char * clsname_v = metricValueClassName(path);
  char * clsname_p = NULL;
  char * pluginname = NULL;

  if (clsname_v) {
    if (_debug)
      fprintf(stderr,"::: requesting definition for %s\n",clsname_v);
    defpath = CMNewObjectPath(broker,NULL,"Linux_MetricValueDefinition",
			      &rc);
    if (defpath) {
      CMSetNameSpaceFromObjectPath(defpath,path);
      CMAddKey(defpath,"MetricValueClassName",clsname_v,CMPI_chars);
      if( _debug )
	fprintf(stderr,"::: getting instance  for %s (%s)\n",clsname_v,
		CMGetCharPtr(CDToString(broker,defpath,NULL)));
      definst = CBGetInstance(broker,context,defpath,NULL,&rc);
      if (definst) {
	if( _debug )
	  fprintf(stderr,"::: got instance for %s\n",clsname_v);
	plugindata = CMGetProperty(definst,"MetricDefinitionClassName",&rc);
	clsname_p = plugindata.value.string ? 
	  CMGetCharPtr(plugindata.value.string) : NULL;
      }
    }
    if (clsname_p) {
      if( _debug )
	fprintf(stderr,"::: requesting definition for %s\n",clsname_p);
      pluginpath = CMNewObjectPath(broker,NULL,"Linux_RepositoryPlugin",
				   &rc);
      if (pluginpath) {
	CMSetNameSpaceFromObjectPath(pluginpath,path);
	CMAddKey(pluginpath,"MetricDefinitionClassName",clsname_p,CMPI_chars);
	if( _debug )
	  fprintf(stderr,"::: getting instance  for %s (%s)\n",clsname_p,
		  CMGetCharPtr(CDToString(broker,pluginpath,NULL)));
	plugininst = CBGetInstance(broker,context,pluginpath,NULL,&rc);
	if (plugininst) {
	  if( _debug )
	    fprintf(stderr,"::: got instance for %s\n",clsname_p);
	  plugindata = CMGetProperty(plugininst,"RepositoryPluginName",&rc);
	  pluginname = plugindata.value.string ? 
	    CMGetCharPtr(plugindata.value.string) : NULL;
	}
      }
    }
  }
  if( _debug )
    fprintf(stderr,"::: plugin name is %s\n",pluginname);
  return pluginname;
}

static char * metricValueClassName(CMPIObjectPath *path)
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

static char * metricValueName(CMPIObjectPath *path)
{
  CMPIData clsname = CMGetKey(path,"InstanceId",NULL);
  if (clsname.value.string)
    return CMGetCharPtr(clsname.value.string);
  else
    return NULL;
}

static CMPIInstance * _makeInst(CMPIBroker * broker, 
				char * defname,
				int    defid,
				ValueItem *val, 
				unsigned   datatype,
				CMPIObjectPath *cop,
				CMPIStatus * rc)
{
  CMPIObjectPath * co;
  CMPIInstance   * ci = NULL;
  char             instid[1000];
  char             valbuf[1000];
  CMPIDateTime   * datetime;
  
  if( _debug )
    fprintf(stderr,"::: make inst for cls %s\n",metricValueClassName(cop));
  co = CMNewObjectPath(broker,NULL,metricValueClassName(cop),rc);
  if (co) {
    CMSetNameSpaceFromObjectPath(co,cop);
    ci = CMNewInstance(broker,co,rc);
    if (ci) {
      if( _debug )
	fprintf(stderr,"::: make inst id=%d %s\n",defid,defname);
      CMSetProperty(ci,"InstanceId",
		    makeInstId(instid,defname,defid,val->viResource,
			       val->viCaptureTime),
		    CMPI_chars);
      CMSetProperty(ci,"MetricDefinitionId",defname,CMPI_chars);
      CMSetProperty(ci,"MeasuredElementName",val->viResource,CMPI_chars);
      datetime = 
	CMNewDateTimeFromBinary(broker,
				(long long)val->viCaptureTime*1000000,
				0, NULL);
      if (datetime)
	CMSetProperty(ci,"TimeStamp",&datetime,CMPI_dateTime);
      datetime = 
	CMNewDateTimeFromBinary(broker,
				(long long)val->viDuration*1000000,
				1, NULL);
      if (datetime)
	CMSetProperty(ci,"Duration",&datetime,CMPI_dateTime);
      switch (datatype) {
      case MD_BOOL:
	sprintf(valbuf,"%s", *val->viValue ? "true" : "false" );
	break;
      case MD_UINT8:
	sprintf(valbuf,"%hhu",*(unsigned short*)val->viValue);
	break;
      case MD_SINT8:
	sprintf(valbuf,"%hhd",*(short*)val->viValue);
	break;
      case MD_UINT16:
      case MD_CHAR16:
	sprintf(valbuf,"%hu",*(unsigned short*)val->viValue);
	break;
      case MD_SINT16:
	sprintf(valbuf,"%hd",*(short*)val->viValue);
	break;
      case MD_UINT32:
	sprintf(valbuf,"%lu",*(unsigned long*)val->viValue);
	break;
      case MD_SINT32:
	sprintf(valbuf,"%ld",*(long*)val->viValue);
	break;
      case MD_UINT64:
	sprintf(valbuf,"%llu",*(unsigned long long*)val->viValue);
	break;
      case MD_SINT64:
	sprintf(valbuf,"%lld",*(long long*)val->viValue);
	break;
      case MD_FLOAT32:
	sprintf(valbuf,"%f",*(float*)val->viValue);
	break;
      case MD_FLOAT64:
	sprintf(valbuf,"%f",*(double*)val->viValue);
	break;
      case MD_STRING:
	strcpy(valbuf,val->viValue);
	break;
      default:
	sprintf(valbuf,"datatype %0x not supported",datatype);
	break;
      }
      CMSetProperty(ci,"MetricValue",valbuf,CMPI_chars);
    }
  }
  return ci;
}

static CMPIObjectPath * _makePath(CMPIBroker * broker,
				  char * defname,
				  int    defid,
				  ValueItem *val, 
				  CMPIObjectPath *cop,
				  CMPIStatus * rc)
{
  CMPIObjectPath * co;
  char             instid[1000];

  if( _debug )
    fprintf(stderr,"::: make path for cls %s\n",metricValueClassName(cop));
  co = CMNewObjectPath(broker,NULL,metricValueClassName(cop),rc);
  if (co) {
    CMSetNameSpaceFromObjectPath(co,cop);
    if( _debug )
      fprintf(stderr,"::: make path id=%d %s\n",defid,defname);
    CMAddKey(co,"InstanceId",
	     makeInstId(instid,defname,defid,val->viResource,
			val->viCaptureTime),
	     CMPI_chars);
    /* TODO: need to add more data (for id, etc.) to value items */
    CMAddKey(co,"MetricDefinitionId",makeDefId(instid,defname,defid),
	     CMPI_chars);
  }
  return co;
}


static char * makeInstId(char * instid, const char * name, int id,  
			       const char * resource, time_t timestamp)
{
  if( _debug )
    fprintf(stderr,"::: building inst id %s.%d.%s.%ld",name,id,
	    resource,timestamp);
  sprintf(instid,"%s.%d.%s.%ld",name,id,resource,timestamp);
  return instid;
}

static char * makeDefId(char * defid, const char * name, int id)  
{
  if( _debug )
    fprintf(stderr,"::: building def id %s.%d",name,id);
  sprintf(defid,"%s.%d",name,id);
  return defid;
}

static int parseInstId(const char * instid,
		       char * name, int * id, char * resource,
		       time_t * timestamp)
{
  char * nextf;
  nextf = strchr(instid,'.');
  if (nextf==NULL) return -1;
  *nextf = 0;
  sscanf(instid,"%s",name);
  instid = nextf + 1;
  nextf = strchr(instid,'.');
  if (nextf==NULL) return -1;
  *nextf = 0;
  sscanf(instid,"%d",id);
  instid = nextf + 1;
  nextf = strrchr(instid,'.');
  if (nextf==NULL) return -1;
  *nextf = 0;
  sscanf(instid,"%s",resource);
  instid = nextf + 1;
  sscanf(instid,"%ld",timestamp);
  return 0;
}

static void returnPathes(CMPIBroker *_broker,
			 RepositoryPluginDefinition *rdef,
			 int rdefnum,
			 COMMHEAP *ch,
			 CMPIObjectPath *ref,
			 CMPIResult *rslt,
			 CMPIStatus *rc)
{
  int i,j;
  ValueRequest vr;
  CMPIObjectPath * co = NULL;
  
  for (i=0; i < rdefnum; i++) {
    vr.vsId = rdef[i].rdId;
    vr.vsResource = NULL;
    vr.vsFrom = vr.vsTo = 0;
    if( _debug )
      fprintf( stderr, "::: getting value for id %d\n", rdef[i].rdId);
    if (rrepos_get(&vr,ch) == 0) {
      if( _debug )
	fprintf( stderr, "::: got %d values for id \n", vr.vsId);
      for (j=0; j< vr.vsNumValues; j++) {
	co = _makePath( _broker, rdef[i].rdName, rdef[i].rdId,
			&vr.vsValues[j], 
			ref, rc );
	if( co == NULL ) {
	  if( _debug ) {
	    if( rc->msg != NULL )
	      { fprintf(stderr,"rc.msg: %s\n",CMGetCharPtr(rc->msg)); }
	  }
	  break;
	}      
	CMReturnObjectPath( rslt, co );
      }
    }
  }
}

static void returnInstances(CMPIBroker *_broker,
			    RepositoryPluginDefinition *rdef,
			    int rdefnum,
			    COMMHEAP *ch,
			    CMPIObjectPath *ref,
			    CMPIResult *rslt,
			    CMPIStatus *rc)
{
  int i,j;
  ValueRequest vr;
  CMPIInstance * ci = NULL;
  
  for (i=0; i < rdefnum; i++) {
    vr.vsId = rdef[i].rdId;
    vr.vsResource = NULL;
    vr.vsFrom = vr.vsTo = 0;
    if( _debug )
      fprintf( stderr, "::: getting value for id %d\n", rdef[i].rdId);
    if (rrepos_get(&vr,ch) == 0) {
      if( _debug )
	fprintf( stderr, "::: got %d values for id \n", vr.vsId);
      for (j=0; j< vr.vsNumValues; j++) {
	ci = _makeInst( _broker, rdef[i].rdName, rdef[i].rdId,
			&vr.vsValues[j], vr.vsDataType,
			ref, rc );
	if( ci == NULL ) {
	  if( _debug ) {
	    if( rc->msg != NULL )
	      { fprintf(stderr,"rc.msg: %s\n",CMGetCharPtr(rc->msg)); }
	  }
	  break;
	}      
	CMReturnInstance( rslt, ci );
      }
    }
  }
}

static CMPIObjectPath * _pathFromName(CMPIBroker * broker,
				      CMPIObjectPath *cop,
				      const char *name,
				      CMPIStatus *rc)
{   
  return CMNewObjectPath(broker,
			 CMGetCharPtr(CMGetNameSpace(cop,NULL)),
			 (char*)name,rc);
}

/* ---------------------------------------------------------------------------*/
/*              end of OSBase_MetricValueProvider                             */
/* ---------------------------------------------------------------------------*/

