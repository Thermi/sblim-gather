/*
 * $Id: OSBase_MetricUtil.c,v 1.2 2004/09/24 15:30:29 mihajlov Exp $
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
 * Author:      Viktor Mihajlovski <mihajlov@de.ibm.com>
 *
 * Description: Utility Functions for Metric Providers
 * 
 */

#include "OSBase_MetricUtil.h"
#include <string.h>
#include <stdio.h>
#include <rrepos.h>
#include <mrwlock.h>
#include <cmpift.h>
#include <cmpimacs.h>

#define PLUGINCLASSNAME "Linux_RepositoryPlugin"
#define VALDEFCLASSNAME "Linux_MetricValueDefinition"


int checkRepositoryConnection()
{
  RepositoryStatus stat;
  if (rrepos_status(&stat)==0 && stat.rsInitialized)
    return 1;
  else
    return 0;
}

MRWLOCK_DEFINE(MdefLock);

static struct _MdefList {
  char * mdef_metricname;
  int    mdef_metricid;
  char * mdef_classname;
  char * mdef_pluginname;
  int    mdef_datatype;
} * metricDefinitionList = NULL;

static struct _MvalList {
  char * mval_classname;
  char * mdef_classname;
} * metricValueList;


/*
 * refresh the cache of metric definition classes
 */

static void removeValueList()
{
  int i;
  /* assume lock is already done */
  if (metricValueList) {
    i=0;
    while(metricValueList[i].mval_classname) {      
      free(metricValueList[i].mval_classname);
      free(metricValueList[i].mdef_classname);
      i++;
    }
    free(metricValueList);
    metricValueList = NULL;
  }
  
}

static void removeDefinitionList()
{
  int i;
  /* assume lock is already done */
  if (metricDefinitionList) {
    /* free string pointers */
    i=0;
    while(metricDefinitionList[i].mdef_metricname) {
      free(metricDefinitionList[i].mdef_metricname);
      free(metricDefinitionList[i].mdef_classname);
      free(metricDefinitionList[i].mdef_pluginname);
      i++;
    }
    free(metricDefinitionList);
    metricDefinitionList = NULL;
  }
}

static int refreshMetricValueList(CMPIBroker * broker, CMPIContext * ctx, 
			      const char *namesp)
{
  CMPIData         data, defdata, valdata;
  CMPIObjectPath  *co = CMNewObjectPath(broker,namesp,VALDEFCLASSNAME,NULL);
  CMPIEnumeration *en = CBEnumInstances(broker,ctx,co,NULL,NULL);
  int              totalnum = 0;

  MWriteLock(&MdefLock);
  removeValueList();
  while (en && CMHasNext(en,NULL)) {
    data = CMGetNext(en,NULL);
    if (data.value.inst) {
      valdata=CMGetProperty(data.value.inst,"MetricValueClassName",NULL);
      defdata=CMGetProperty(data.value.inst,"MetricDefinitionClassName",
			    NULL);
    }
    if (valdata.value.string && defdata.value.string) {
      metricValueList = realloc(metricValueList,
				sizeof(struct _MvalList)*(totalnum+2));
      metricValueList[totalnum].mval_classname = 
	strdup(CMGetCharPtr(valdata.value.string));
      metricValueList[totalnum].mdef_classname = 
	strdup(CMGetCharPtr(defdata.value.string));
      totalnum++;
      metricValueList[totalnum].mval_classname = NULL; /* End of list */
    } else {
      MWriteUnlock(&MdefLock);
      return -1;
    }
  }
  MWriteUnlock(&MdefLock);
  return 0;
}

int refreshMetricDefClasses(CMPIBroker * broker, CMPIContext * ctx, 
			    const char *namesp)
     /* TODO: rework error handling */
{
  CMPIData         data, plugindata;
  CMPIObjectPath  *co = CMNewObjectPath(broker,namesp,PLUGINCLASSNAME,NULL);
  CMPIEnumeration *en = CBEnumInstances(broker,ctx,co,NULL,NULL);
  char            *pname;
  int              rdefnum, i, totalnum;
  RepositoryPluginDefinition *rdef;
  COMMHEAP         ch;

  /* refresh definition/value mapping list */
  if (refreshMetricValueList(broker,ctx,namesp)) {
    return -1;
  }

  /* check for repository connection */
  if (checkRepositoryConnection()==0) {
    return -1;
  }
  ch=ch_init();

  MWriteLock(&MdefLock);
  removeDefinitionList();
  /* loop over all repository plugins to get to the metric id to
     classname mappings */
  totalnum=0;
  while (en && CMHasNext(en,NULL)) {
    data = CMGetNext(en,NULL);
    if (data.value.inst) {
      plugindata=CMGetProperty(data.value.inst,"RepositoryPluginName",NULL);
      data = CMGetProperty(data.value.inst,"MetricDefinitionClassName",NULL);
      if (plugindata.value.string && data.value.string) {	
	pname = CMGetCharPtr(plugindata.value.string);
	rdefnum = rreposplugin_list(pname,&rdef,ch);
	metricDefinitionList = realloc(metricDefinitionList,
				       (rdefnum+totalnum+1)*sizeof(struct _MdefList));
	for (i=0;i<rdefnum;i++) {
	  metricDefinitionList[totalnum+i].mdef_metricname = strdup(rdef[i].rdName);
	  metricDefinitionList[totalnum+i].mdef_metricid = rdef[i].rdId;
	  metricDefinitionList[totalnum+i].mdef_classname = 
	    strdup(CMGetCharPtr(data.value.string));
	  metricDefinitionList[totalnum+i].mdef_pluginname = strdup(pname);
	  metricDefinitionList[totalnum+i].mdef_datatype = rdef[i].rdDataType;
	}
	/* identify last element with null name */
	totalnum += rdefnum;
	metricDefinitionList[totalnum].mdef_metricname = NULL;
      }
    }
  }
  MWriteUnlock(&MdefLock);
  ch_release(ch);
  return 0;
}

void releaseMetricDefClasses()
{
  MWriteLock(&MdefLock);
  if (metricDefinitionList) {
    removeDefinitionList();
  }
  if (metricValueList) {
    removeValueList();
  }
  MWriteUnlock(&MdefLock);
}

static int metricDefClassIndex(CMPIBroker *broker, CMPIContext *ctx, 
		       const char *namesp,const char *name, int id)
{
  int i=0;
  if (metricDefinitionList==NULL) {
    refreshMetricDefClasses(broker,ctx,namesp);
  }

  MReadLock(&MdefLock);
  while (metricDefinitionList && metricDefinitionList[i].mdef_metricname) {
    if (strcmp(name,metricDefinitionList[i].mdef_metricname) == 0 &&
	id == metricDefinitionList[i].mdef_metricid) {
      MReadUnlock(&MdefLock);
      return i;
    }
    i++;
  }
  MReadUnlock(&MdefLock);
  return -1;
}

int metricDefClassName(CMPIBroker *broker, CMPIContext *ctx, 
		       const char *namesp,char *clsname, 
		       const char *name, int id)
{
  int i=0;
  if (metricDefinitionList==NULL) {
    refreshMetricDefClasses(broker,ctx,namesp);
  }

  MReadLock(&MdefLock);
  while (metricDefinitionList && metricDefinitionList[i].mdef_metricname) {
    if (strcmp(name,metricDefinitionList[i].mdef_metricname) == 0 &&
	id == metricDefinitionList[i].mdef_metricid) {
      strcpy(clsname,metricDefinitionList[i].mdef_classname);
      MReadUnlock(&MdefLock);
      return 0;
    }
    i++;
  }
  MReadUnlock(&MdefLock);
  return -1;
}

int metricPluginName(CMPIBroker *broker, CMPIContext *ctx, 
		     const char *namesp,char *pluginname, 
		     const char *name, int id)
{
  int i=0;
  if (metricDefinitionList==NULL) {
    refreshMetricDefClasses(broker,ctx,namesp);
  }

  MReadLock(&MdefLock);
  while (metricDefinitionList && metricDefinitionList[i].mdef_metricname) {
    if (strcmp(name,metricDefinitionList[i].mdef_metricname) == 0 &&
	id == metricDefinitionList[i].mdef_metricid) {
      strcpy(pluginname,metricDefinitionList[i].mdef_pluginname);
      MReadUnlock(&MdefLock);
      return 0;
    }
    i++;
  }
  MReadUnlock(&MdefLock);
  return -1;
}

int metricValueClassName(CMPIBroker *broker, CMPIContext *ctx, 
			 const char *namesp,char *clsname, 
			 const char *name, int id)
{
  int  i=0;
  char defname[500];
  
  if (metricDefClassName(broker,ctx,namesp,defname,name,id)) {
    return -1;
  }
  
  MReadLock(&MdefLock);
  while (metricValueList && metricValueList[i].mval_classname) {
    if (strcmp(defname,metricValueList[i].mdef_classname)==0){
      strcpy(clsname,metricValueList[i].mval_classname);
      MReadUnlock(&MdefLock);
      return 0;
    }
    i++;
  }
  MReadUnlock(&MdefLock);
  return -1;
}

char * makeMetricDefId(char * defid, const char * name, int id)
{
  /* TODO: stupid syntax, no escapes - revise */
  sprintf(defid,"%s.%d",name,id);
  return defid;
}

int parseMetricDefId(const char * defid,
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

char * makeMetricValueId(char * instid, const char * name, int id,  
				const char * resource, time_t timestamp)
{
  /* TODO: stupid syntax, no escapes - revise */
  sprintf(instid,"%s.%d.%s.%ld",name,id,resource,timestamp);
  return instid;
}

int parseMetricValueId(const char * instid,
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

static char * metricClassName(const CMPIObjectPath *path)
{
  /* return class name or NULL if CIM_ (base) class */
  CMPIStatus rc;
  CMPIString * clsname = CMGetClassName(path,&rc);

  if (clsname) {
    return CMGetCharPtr(clsname);
  } else {
    return NULL;
  }
}

int pluginForClass(CMPIBroker *broker, CMPIContext *ctx, 
		   const CMPIObjectPath *cop, char *pluginname)
{
  char *metricclass = metricClassName(cop);
  int   i=0;

  if (metricDefinitionList==NULL) {
    refreshMetricDefClasses(broker,ctx,CMGetCharPtr(CMGetNameSpace(cop,NULL)));
  }

  MReadLock(&MdefLock);
  while(metricDefinitionList && metricDefinitionList[i].mdef_metricname) {
    if (strcmp(metricclass,metricDefinitionList[i].mdef_classname)==0) {
      strcpy(pluginname,metricDefinitionList[i].mdef_pluginname);
      MReadUnlock(&MdefLock);
      return 0;
    }
    i++;
  }
  MReadUnlock(&MdefLock);
  return -1;
}

int getMetricDefsForClass(CMPIBroker *broker, CMPIContext *ctx, 
			  const CMPIObjectPath* cop,
			  char ***mnames, int **mids)
{
  char pluginname[500];
  int  i=0;
  int  totalnum=0;
  
  if (mnames && mids) {
    *mnames=NULL;
    *mids=NULL;
    if (pluginForClass(broker,ctx,cop,pluginname)==0) {
      /* search for given plugin name */
      MReadLock(&MdefLock);
      while (metricDefinitionList && metricDefinitionList[i].mdef_metricname) {
	if (strcmp(metricDefinitionList[i].mdef_pluginname,pluginname)==0) {
	  *mnames = realloc(*mnames,(totalnum+2)*sizeof(char*));
	  *mids = realloc(*mids,(totalnum+1)*sizeof(int));
	  (*mnames)[totalnum] = strdup(metricDefinitionList[i].mdef_metricname);
	  (*mnames)[totalnum+1] = NULL;
	  (*mids)[totalnum] = metricDefinitionList[i].mdef_metricid;
	  totalnum++;
	}
	i++;
      }
      MReadUnlock(&MdefLock);
    } else {
      /* return all - should only happen for polymorphic call in SNIA CIMOM */
      MReadLock(&MdefLock);
      while (metricDefinitionList && 
	     metricDefinitionList[totalnum].mdef_metricname) {
	*mnames = realloc(*mnames,(totalnum+2)*sizeof(char*));
	*mids = realloc(*mids,(totalnum+1)*sizeof(int));
	(*mnames)[totalnum] = 
	  strdup(metricDefinitionList[totalnum].mdef_metricname);
	(*mnames)[totalnum+1] = NULL;
	(*mids)[totalnum] = metricDefinitionList[totalnum].mdef_metricid;
	totalnum++;
      }
      MReadUnlock(&MdefLock);     
    }
  }
  return totalnum;
}

void releaseMetricDefs(char **mnames,int *mids)
{
  int i=0;
  if (mnames) {
    while(mnames[i]) {
      free(mnames[i]);
      i++;
    }
    free(mnames);
  }
  if (mids) {
    free(mids);
  }
}

CMPIInstance * makeMetricValueInst(CMPIBroker * broker, 
				   CMPIContext * ctx,
				   const char * defname,
				   int    defid,
				   const ValueItem *val, 
				   unsigned   datatype,
				   const CMPIObjectPath *cop,
				   CMPIStatus * rc)
{
  CMPIObjectPath * co;
  CMPIInstance   * ci = NULL;
  char             instid[1000];
  char             valbuf[1000];
  char             valclsname[1000];
  char           * namesp;
  CMPIDateTime   * datetime;

  namesp=CMGetCharPtr(CMGetNameSpace(cop,NULL));
  if (metricValueClassName(broker,ctx,namesp,valclsname,defname,defid)) {
    return NULL;
  }
  
  co = CMNewObjectPath(broker,namesp,valclsname,rc);
  if (co) {
    ci = CMNewInstance(broker,co,rc);
    if (ci) {
      CMSetProperty(ci,"InstanceId",
		    makeMetricValueId(instid,defname,defid,val->viResource,
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

CMPIObjectPath * makeMetricValuePath(CMPIBroker * broker,
				     CMPIContext * ctx,
				     const char * defname,
				     int    defid,
				     const ValueItem *val, 
				     const CMPIObjectPath *cop,
				     CMPIStatus * rc)
{
  CMPIObjectPath * co;
  char           * namesp;
  char             instid[1000];
  char             valclsname[1000];

  namesp=CMGetCharPtr(CMGetNameSpace(cop,NULL));
  if (metricValueClassName(broker,ctx,namesp,valclsname,defname,defid)) {
    return NULL;
  }
  co = CMNewObjectPath(broker,namesp,valclsname,rc);
  if (co) {
    CMAddKey(co,"InstanceId",
	     makeMetricValueId(instid,defname,defid,val->viResource,
			       val->viCaptureTime),
	     CMPI_chars);
    /* TODO: need to add more data (for id, etc.) to value items */
    CMAddKey(co,"MetricDefinitionId",makeMetricDefId(instid,defname,defid),
	     CMPI_chars);
  }
  return co;
}

CMPIObjectPath * makeMetricDefPath(CMPIBroker * broker,
				   CMPIContext * ctx,
				   const char * defname,
				   int    defid,
				   const char *namesp,
				   CMPIStatus * rc)
{
  CMPIObjectPath * co;
  char             instid[1000];
  char             defclsname[1000];

  if (metricDefClassName(broker,ctx,namesp,defclsname,defname,defid)) {
    return NULL;
  }
  co = CMNewObjectPath(broker,namesp,defclsname,rc);
  if (co) {
    CMAddKey(co,"Id",
	     makeMetricDefId(instid,defname,defid),
	     CMPI_chars);
  }
  return co;
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

CMPIInstance * makeMetricDefInst(CMPIBroker * broker,
				 CMPIContext * ctx,
				 const char * defname,
				 int    defid,
				 const char *namesp,
				 CMPIStatus * rc)
{
  CMPIObjectPath * co;
  CMPIInstance   * ci = NULL;
  char             instid[500];
  short            dt;
  int              i;

  i = metricDefClassIndex(broker,ctx,namesp,defname,defid);
  if (i < 0) {
    return NULL;
  }
  co = CMNewObjectPath(broker,namesp,metricDefinitionList[i].mdef_classname,rc);
  
  /* get plugin's metric definition */
  if (co) {
    ci = CMNewInstance(broker,co,rc);
    if (ci) {
      CMSetProperty(ci,"Id",
		    makeMetricDefId(instid,defname,defid),
		    CMPI_chars);
      CMSetProperty(ci,"Name",defname,CMPI_chars);
      for (dt=0;dt<sizeof(typetable)/sizeof(int);dt++) {
	if (metricDefinitionList[i].mdef_datatype==typetable[dt])
	  break;
      }
      if (dt<sizeof(typetable)/sizeof(int))
	CMSetProperty(ci,"DataType",&dt,CMPI_uint16);
      return ci;
    }
  }
  return NULL;
}
