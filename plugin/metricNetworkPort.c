/*
 * $Id: metricNetworkPort.c,v 1.1 2003/10/17 13:56:01 mihajlov Exp $
 *
 * (C) Copyright IBM Corp. 2003
 *
 * THIS FILE IS PROVIDED UNDER THE TERMS OF THE COMMON PUBLIC LICENSE
 * ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
 * CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
 *
 * You can obtain a current copy of the Common Public License from
 * http://oss.software.ibm.com/developerworks/opensource/license-cpl.html
 *
 * Author:       Heidi Neumann <heidineu@de.ibm.com>
 * Contributors: 
 *
 * Description:
 * This shared library is a Plugin to the metrics gatherer written
 * by Viktor Mihajlovski <mihajlov@de.ibm.com>, and offers the 
 * following Processor specific metrics :
 *
 * BytesSubmitted
 * BytesTransmitted
 * BytesReceived
 * ErrorRate
 *
 */

/* ---------------------------------------------------------------------------*/

#include <mplugin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     
#include <time.h>

/* ---------------------------------------------------------------------------*/

static MetricDefinition  metricDef[4];

static ResourceLister          resourceLister;
static ResourceListDeallocator resourceListDeallocator;

/* --- BytesSubmitted is base for :
 * BytesTransmitted, BytesReceived, ErrorRate --- */
static MetricRetriever   metricRetrBytesSubmitted;
static MetricCalculator  metricCalcBytesSubmitted;

static MetricCalculator  metricCalcBytesTransmitted;
static MetricCalculator  metricCalcBytesReceived;
static MetricCalculator  metricCalcErrorRate;

/* ---------------------------------------------------------------------------*/

#define NETINFO "/proc/net/dev"

static char * _enum_port = NULL;
static int    _enum_size = 0;

static int enum_all_port();

/* ---------------------------------------------------------------------------*/

int _DefinedMetrics ( MetricRegisterId *mr,
		      const char * pluginname,
		      size_t *mdnum,
		      MetricDefinition **md ) {

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Retrieving metric definitions\n",
	  __FILE__,__LINE__);
#endif
  if (mr==NULL||mdnum==NULL||md==NULL) {
    fprintf(stderr,"--- %s(%i) : invalid parameter list\n",__FILE__,__LINE__);
    return -1;
  }

  metricDef[0].mdName="BytesSubmitted";
  metricDef[0].mdId=mr(pluginname,metricDef[0].mdName);
  metricDef[0].mdSampleInterval=60;
  metricDef[0].mdMetricType=MD_RETRIEVED|MD_POINT;
  metricDef[0].mdDataType=MD_STRING;
  metricDef[0].mproc=metricRetrBytesSubmitted;
  metricDef[0].mdeal=free;
  metricDef[0].mcalc=metricCalcBytesSubmitted;
  metricDef[0].mresl=resourceLister;
  metricDef[0].mresldeal=resourceListDeallocator;

  metricDef[1].mdName="BytesTransmitted";
  metricDef[1].mdId=mr(pluginname,metricDef[1].mdName);
  metricDef[1].mdMetricType=MD_CALCULATED|MD_INTERVAL;
  metricDef[1].mdDataType=MD_UINT64;
  metricDef[1].mdAliasId=metricDef[0].mdId;
  metricDef[1].mproc=NULL;
  metricDef[1].mdeal=NULL;
  metricDef[1].mcalc=metricCalcBytesTransmitted;
  metricDef[1].mresl=metricDef[0].mresl;
  metricDef[1].mresldeal=metricDef[0].mresldeal;

  metricDef[2].mdName="BytesReceived";
  metricDef[2].mdId=mr(pluginname,metricDef[2].mdName);
  metricDef[2].mdMetricType=MD_CALCULATED|MD_INTERVAL;
  metricDef[2].mdDataType=MD_UINT64;
  metricDef[2].mdAliasId=metricDef[0].mdId;
  metricDef[2].mproc=NULL;
  metricDef[2].mdeal=NULL;
  metricDef[2].mcalc=metricCalcBytesReceived;
  metricDef[2].mresl=metricDef[0].mresl;
  metricDef[2].mresldeal=metricDef[0].mresldeal;

  metricDef[3].mdName="ErrorRate";
  metricDef[3].mdId=mr(pluginname,metricDef[3].mdName);
  metricDef[3].mdMetricType=MD_CALCULATED|MD_RATE;
  metricDef[3].mdDataType=MD_FLOAT32;
  metricDef[3].mdAliasId=metricDef[0].mdId;
  metricDef[3].mproc=NULL;
  metricDef[3].mdeal=NULL;
  metricDef[3].mcalc=metricCalcErrorRate;
  metricDef[3].mresl=metricDef[0].mresl;
  metricDef[3].mresldeal=metricDef[0].mresldeal;

  *mdnum=4;
  *md=metricDef;
  return 0;
}

int _StartStopMetrics (int starting) {

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : %s metric processing\n",
	  __FILE__,__LINE__,starting?"Starting":"Stopping");
#endif  
  if( starting ) {
#ifdef DEBUG
    fprintf(stderr,"--- %s(%i) : enumerate network ports\n",
	    __FILE__,__LINE__);
#endif
    if( enum_all_port() != 0 ) { return -1; }
  }
  else {
#ifdef DEBUG
    fprintf(stderr,"--- %s(%i) : free network port entries\n",
	    __FILE__,__LINE__);
#endif
    if(_enum_port) free(_enum_port);
  }
  
  return 0;
}

int resourceLister(int mid, char *** list) {
  int i = 0;
  if (list) { 
    *list = calloc(_enum_size+1,sizeof(char*));
    for(;i<_enum_size;i++) {
      (*list)[i] = strdup(_enum_port + (i*64));
    }
    return _enum_size;
  }
  return -1;
}

void resourceListDeallocator(char ** list) {
  char ** ls = list;
  while(*ls) {
    free(*ls);
    ls++;
  }
  free(list);
}



/* ---------------------------------------------------------------------------*/

/* 
 * The raw data BytesSubmitted has the following syntax :
 *
 * <bytes received>:<bytes transmitted>:
 * <errors received>:<errors transmitted>:
 * <packtes received>:<packets transmitted>
 *
 */


int metricRetrBytesSubmitted( int mid, 
			      MetricReturner mret ) {  
  MetricValue * mv  = NULL;
  FILE * fhd        = NULL;
  char * ptr        = NULL;
  char * end        = NULL;
  char * port       = NULL;
  char   buf[60000];
  char   values[(6*sizeof(unsigned long long))+6];
  size_t bytes_read = 0;
  int    i          = 0;
  unsigned long long receive_byte, receive_packets, receive_error = 0;
  unsigned long long trans_byte, trans_packets, trans_error       = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Retrieving BytesSubmitted\n",
	  __FILE__,__LINE__);
#endif  
  if (mret==NULL) { fprintf(stderr,"Returner pointer is NULL\n"); }
  else {

#ifdef DEBUG
    fprintf(stderr,"--- %s(%i) : Sampling for metric BytesSubmitted ID %d\n",
	    __FILE__,__LINE__,mid); 
#endif    
    if( _enum_size > 0 ) {
      
      if( (fhd = fopen(NETINFO,"r")) != NULL ) {
	bytes_read = fread(buf, 1, sizeof(buf)-1, fhd);
	if( bytes_read > 0 ) {
	  ptr = strchr(buf,'\n')+1;
	  ptr = strchr(ptr,'\n')+1;
	  
	  for(;i<_enum_size;i++) {
	    end = strchr(ptr,'\n');
	    sscanf(ptr,
		   "%*s %lld %lld %lld %*s %*s %*s %*s %*s %lld %lld %lld",
		   &receive_byte,&receive_packets,&receive_error,
		   &trans_byte,&trans_packets,&trans_error);
	    
	    port = _enum_port + (i*64);
	    //fprintf(stderr,"[%i] port: %s \n",i,port);
	    
	    memset(values,0,sizeof(values));
	    sprintf(values,"%lld:%lld:%lld:%lld:%lld:%lld",
		    receive_byte,trans_byte,receive_error,trans_error,receive_packets,trans_packets);
	    //fprintf(stderr,"values : %s\n",values);
	    
	    mv = calloc(1, sizeof(MetricValue) + 
			(strlen(values)+1) + 
			(strlen(port)+1) );
	    if (mv) {
	      mv->mvId = mid;
	      mv->mvTimeStamp = time(NULL);
	      mv->mvDataType = MD_STRING;
	      mv->mvDataLength = (strlen(values)+1);
	      mv->mvData = (void*)mv + sizeof(MetricValue);
	      strcpy(mv->mvData,values);
	      mv->mvResource = (void*)mv + sizeof(MetricValue) + (strlen(values)+1);
	      strcpy(mv->mvResource,port);
	      mret(mv);
	    } 
	    ptr = end+1;
	  }
	}
	fclose(fhd);
      }
      else { return -1; }
    }
    return _enum_size;
  }
  return -1;
}


size_t metricCalcBytesSubmitted( MetricValue *mv,   
				 int mnum,
				 void *v, 
				 size_t vlen ) {

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate BytesSubmitted\n",
	  __FILE__,__LINE__);
#endif
  /* plain copy */
  if (mv && (vlen>=mv->mvDataLength) && (mnum==1) ) {
    memcpy(v,mv->mvData,mv->mvDataLength);
    return mv->mvDataLength;
  }
  return -1;
}


size_t metricCalcBytesTransmitted( MetricValue *mv,   
				   int mnum,
				   void *v, 
				   size_t vlen ) {
  char * hlp = NULL;
  char * end = NULL;
  char   bytes[sizeof(unsigned long long)+1];
  unsigned long long bt = 0;
  unsigned long long b1 = 0;
  unsigned long long b2 = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate BytesTransmitted\n",
	  __FILE__,__LINE__);
#endif  
  if ( mv && (vlen>=sizeof(unsigned long long)) && (mnum>=1) ) {
    hlp = strchr(mv[0].mvData, ':')+1;
    end = strchr(hlp, ':');
    memset(bytes,0,sizeof(bytes));
    strncpy(bytes, hlp, (strlen(hlp)-strlen(end)) );
    b1 = atoll(bytes);

    if( mnum > 1 ) {
      hlp = strchr(mv[mnum-1].mvData,':')+1;
      end = strchr(hlp, ':');
      memset(bytes,0,sizeof(bytes));
      strncpy(bytes, hlp, (strlen(hlp)-strlen(end)) );
      b2 = atoll(bytes);
      
      bt = (b1-b2)/2;
    }
    else { bt = b1; }

    //fprintf(stderr,"bytes transmitted: %lld\n",bt);
    memcpy(v,&bt,sizeof(unsigned long long));
    return sizeof(unsigned long long);
  }
  return -1;
}


size_t metricCalcBytesReceived( MetricValue *mv,   
				int mnum,
				void *v, 
				size_t vlen ) {
  char * hlp = NULL;
  char * end = NULL;
  char   bytes[sizeof(unsigned long long)+1];
  unsigned long long br = 0;
  unsigned long long b1 = 0;
  unsigned long long b2 = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate BytesReceived\n",
	  __FILE__,__LINE__);
#endif
  if ( mv && (vlen>=sizeof(unsigned long long)) && (mnum>=1) ) {
    hlp = mv[0].mvData;
    end = strchr(hlp, ':');
    memset(bytes,0,sizeof(bytes));
    strncpy(bytes, hlp, (strlen(hlp)-strlen(end)) );
    b1 = atoll(bytes);

    if( mnum > 1 ) {
      hlp = mv[mnum-1].mvData;
      end = strchr(hlp, ':');
      memset(bytes,0,sizeof(bytes));
      strncpy(bytes, hlp, (strlen(hlp)-strlen(end)) );
      b2 = atoll(bytes);
      
      br = (b1-b2)/2;
    }
    else { br = b1; }

    //fprintf(stderr,"bytes received: %lld\n",bt);
    memcpy(v,&br,sizeof(unsigned long long));
    return sizeof(unsigned long long);
  }
  return -1;
}


size_t metricCalcErrorRate( MetricValue *mv,   
			    int mnum,
			    void *v, 
			    size_t vlen ) {
  char * hlp = NULL;
  char * end = NULL;
  char   errors[sizeof(float)+1];
  float  er = 0;
  float  r1 = 0;
  float  r2 = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate ErrorRate\n",
	  __FILE__,__LINE__);
#endif  
  if ( mv && (vlen>=sizeof(float)) && (mnum>=2) ) {
    hlp = strchr(mv[0].mvData, ':')+1;
    hlp = strchr(hlp, ':')+1;
    end = strchr(hlp, ':');
    memset(errors,0,sizeof(errors));
    strncpy(errors, hlp, (strlen(hlp)-strlen(end)) );
    r1 = atof(errors);
    hlp = strchr(hlp, ':')+1;
    end = strchr(hlp, ':');
    memset(errors,0,sizeof(errors));
    strncpy(errors, hlp, (strlen(hlp)-strlen(end)) );
    r1 = r1 + atof(errors);

    hlp = strchr(mv[mnum-1].mvData,':')+1;
    hlp = strchr(hlp, ':')+1;
    end = strchr(hlp, ':');
    memset(errors,0,sizeof(errors));
    strncpy(errors, hlp, (strlen(hlp)-strlen(end)) );
    r2 = atof(errors);
    hlp = strchr(hlp, ':')+1;
    end = strchr(hlp, ':');
    strncpy(errors, hlp, (strlen(hlp)-strlen(end)) );
    memset(errors,0,sizeof(errors));
    r2 = r2 + atof(errors);
      
    er = (r1-r2) / (mv[0].mvTimeStamp - mv[mnum-1].mvTimeStamp);

    //fprintf(stderr,"error rate: %f\n",er);
    memcpy(v,&er,sizeof(float));
    return sizeof(float);
  }
  return -1;
}



/* ---------------------------------------------------------------------------*/
// get all network port instances
/* ---------------------------------------------------------------------------*/

int enum_all_port() {  

  FILE * fhd        = NULL;
  char * ptr        = NULL;
  char * end        = NULL;
  char   buf[60000];
  char   port[64];
  size_t bytes_read = 0;
  int    i          = 0;


#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : enum_all_port()\n",__FILE__,__LINE__);
#endif
  /* search for network port entries in NETINFO */

  if( (fhd = fopen(NETINFO,"r")) != NULL ) {
    
    bytes_read = fread(buf, 1, sizeof(buf)-1, fhd);
    if( bytes_read > 0 ) {

      ptr = strchr(buf,'\n')+1;
      ptr = strchr(ptr,'\n')+1;
      
      i=0;
      _enum_size = 1;
      _enum_port = calloc(_enum_size,64);

      do {

	if(strlen(ptr) == 1) { break; }

	if( i==_enum_size ) {
	  _enum_size++;
	  _enum_port = realloc(_enum_port,_enum_size*64);
	  memset(_enum_port + (i*64), 0, 64);
	}
	
	memset(port,0,sizeof(port));
	sscanf(ptr,"%s",port);	
	end = strchr(port,':');
	*end = '\0';
	strcpy(_enum_port + (i*64),port);
	i++;
	ptr++;
      }
      while( (ptr = strchr(ptr,'\n')) != NULL );
    }
    fclose(fhd);
  }
  else { return -1; }
  
  return 0;
}

/* ---------------------------------------------------------------------------*/
/*                          end of metricProcessor.c                          */
/* ---------------------------------------------------------------------------*/
