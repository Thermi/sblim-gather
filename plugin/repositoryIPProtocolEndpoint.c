/*
 * $Id: repositoryIPProtocolEndpoint.c,v 1.1 2004/09/15 11:31:05 heidineu Exp $
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
 * Contributors: 
 *
 * Description:
 * Repository Plugin of the following IP Protocol Endpoint specific metrics :
 *
 * BytesSubmitted
 * BytesTransmitted
 * BytesReceived
 * ErrorRate
 * PacketsTransmitted
 * PacketsReceived
 *
 */

/* ---------------------------------------------------------------------------*/

#include <mplugin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------------------------------------------------------------------------*/

static MetricCalculationDefinition metricCalcDef[6];

/* --- BytesSubmitted is base for :
 * BytesTransmitted, BytesReceived, ErrorRate,
 * PacketsTransmitted, PacketsReceived
 * --- */
static MetricCalculator  metricCalcBytesSubmitted;
static MetricCalculator  metricCalcBytesTransmitted;
static MetricCalculator  metricCalcBytesReceived;
static MetricCalculator  metricCalcErrorRate;
static MetricCalculator  metricCalcPacketsTransmitted;
static MetricCalculator  metricCalcPacketsReceived;

/* ---------------------------------------------------------------------------*/

static unsigned long long ip_getBytesTransmitted( char * data );
static unsigned long long ip_getBytesReceived( char * data );
static unsigned long long ip_getPacketsTransmitted( char * data );
static unsigned long long ip_getPacketsReceived( char * data );
static unsigned long long ip_getErrorsTransmitted( char * data );
static unsigned long long ip_getErrorsReceived( char * data );

/* ---------------------------------------------------------------------------*/

int _DefinedRepositoryMetrics( MetricRegisterId *mr,
			       const char *pluginname,
			       size_t *mcnum,
			       MetricCalculationDefinition **mc ) {
#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Retrieving metric calculation definitions\n",
	  __FILE__,__LINE__);
#endif
  if (mr==NULL||mcnum==NULL||mc==NULL) {
    fprintf(stderr,"--- %s(%i) : invalid parameter list\n",__FILE__,__LINE__);
    return -1;
  }

  metricCalcDef[0].mcVersion=MD_VERSION;
  metricCalcDef[0].mcName="BytesSubmitted";
  metricCalcDef[0].mcId=mr(pluginname,metricCalcDef[0].mcName);
  metricCalcDef[0].mcMetricType=MD_RETRIEVED|MD_POINT;
  metricCalcDef[0].mcDataType=MD_STRING;
  metricCalcDef[0].mcCalc=metricCalcBytesSubmitted;

  metricCalcDef[1].mcVersion=MD_VERSION;
  metricCalcDef[1].mcName="BytesTransmitted";
  metricCalcDef[1].mcId=mr(pluginname,metricCalcDef[1].mcName);
  metricCalcDef[1].mcMetricType=MD_CALCULATED|MD_INTERVAL;
  metricCalcDef[1].mcDataType=MD_UINT64;
  metricCalcDef[1].mcAliasId=metricCalcDef[0].mcId;
  metricCalcDef[1].mcCalc=metricCalcBytesTransmitted;

  metricCalcDef[2].mcVersion=MD_VERSION;
  metricCalcDef[2].mcName="BytesReceived";
  metricCalcDef[2].mcId=mr(pluginname,metricCalcDef[2].mcName);
  metricCalcDef[2].mcMetricType=MD_CALCULATED|MD_INTERVAL;
  metricCalcDef[2].mcDataType=MD_UINT64;
  metricCalcDef[2].mcAliasId=metricCalcDef[0].mcId;
  metricCalcDef[2].mcCalc=metricCalcBytesReceived;

  metricCalcDef[3].mcVersion=MD_VERSION;
  metricCalcDef[3].mcName="ErrorRate";
  metricCalcDef[3].mcId=mr(pluginname,metricCalcDef[3].mcName);
  metricCalcDef[3].mcMetricType=MD_CALCULATED|MD_RATE;
  metricCalcDef[3].mcDataType=MD_FLOAT32;
  metricCalcDef[3].mcAliasId=metricCalcDef[0].mcId;
  metricCalcDef[3].mcCalc=metricCalcErrorRate;

  metricCalcDef[4].mcVersion=MD_VERSION;
  metricCalcDef[4].mcName="PacketsTransmitted";
  metricCalcDef[4].mcId=mr(pluginname,metricCalcDef[4].mcName);
  metricCalcDef[4].mcMetricType=MD_CALCULATED|MD_INTERVAL;
  metricCalcDef[4].mcDataType=MD_UINT64;
  metricCalcDef[4].mcAliasId=metricCalcDef[0].mcId;
  metricCalcDef[4].mcCalc=metricCalcPacketsTransmitted;

  metricCalcDef[5].mcVersion=MD_VERSION;
  metricCalcDef[5].mcName="PacketsReceived";
  metricCalcDef[5].mcId=mr(pluginname,metricCalcDef[5].mcName);
  metricCalcDef[5].mcMetricType=MD_CALCULATED|MD_INTERVAL;
  metricCalcDef[5].mcDataType=MD_UINT64;
  metricCalcDef[5].mcAliasId=metricCalcDef[0].mcId;
  metricCalcDef[5].mcCalc=metricCalcPacketsReceived;

  *mcnum=6;
  *mc=metricCalcDef;
  return 0;
}


/* ---------------------------------------------------------------------------*/
/* BytesSubmitted                                                             */
/* ---------------------------------------------------------------------------*/

/* 
 * The raw data BytesSubmitted has the following syntax :
 *
 * <bytes received>:<bytes transmitted>:
 * <errors received>:<errors transmitted>:
 * <packtes received>:<packets transmitted>
 *
 */

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


/* ---------------------------------------------------------------------------*/
/* BytesTransmitted                                                           */
/* ---------------------------------------------------------------------------*/

size_t metricCalcBytesTransmitted( MetricValue *mv,   
				   int mnum,
				   void *v, 
				   size_t vlen ) {
  unsigned long long bt = 0;
  unsigned long long b1 = 0;
  unsigned long long b2 = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate BytesTransmitted\n",
	  __FILE__,__LINE__);
#endif  
  if ( mv && (vlen>=sizeof(unsigned long long)) && (mnum>=1) ) {

    b1 = ip_getBytesTransmitted(mv[0].mvData);
    if( mnum > 1 ) {
      b2 = ip_getBytesTransmitted(mv[mnum-1].mvData);
      bt = b1-b2;
    }
    else { bt = b1; }

    //fprintf(stderr,"bytes transmitted: %lld\n",bt);
    memcpy(v,&bt,sizeof(unsigned long long));
    return sizeof(unsigned long long);
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/
/* BytesReceived                                                              */
/* ---------------------------------------------------------------------------*/

size_t metricCalcBytesReceived( MetricValue *mv,   
				int mnum,
				void *v, 
				size_t vlen ) {
  unsigned long long br = 0;
  unsigned long long b1 = 0;
  unsigned long long b2 = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate BytesReceived\n",
	  __FILE__,__LINE__);
#endif
  if ( mv && (vlen>=sizeof(unsigned long long)) && (mnum>=1) ) {

    b1 = ip_getBytesReceived(mv[0].mvData);
    if( mnum > 1 ) {
      b2 = ip_getBytesReceived(mv[mnum-1].mvData);
      br = b1-b2;
    }
    else { br = b1; }

    //fprintf(stderr,"bytes received: %lld\n",br);
    memcpy(v,&br,sizeof(unsigned long long));
    return sizeof(unsigned long long);
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/
/* ErrorRate                                                                  */
/* ---------------------------------------------------------------------------*/

size_t metricCalcErrorRate( MetricValue *mv,   
			    int mnum,
			    void *v, 
			    size_t vlen ) {
  float rate = 0;
  float et1  = 0;
  float et2  = 0;
  float er1  = 0;
  float er2  = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate ErrorRate\n",
	  __FILE__,__LINE__);
#endif  
  if ( mv && (vlen>=sizeof(float)) && (mnum>=2) ) {

    et1 = ip_getErrorsTransmitted(mv[0].mvData);
    er1 = ip_getErrorsReceived(mv[0].mvData);

    et2 = ip_getErrorsTransmitted(mv[mnum-1].mvData);
    er2 = ip_getErrorsReceived(mv[mnum-1].mvData);

    rate = ( (et1+er1) - (et2+er2) ) /
           (mv[0].mvTimeStamp - mv[mnum-1].mvTimeStamp);

    //fprintf(stderr,"error rate: %f\n",rate);
    memcpy(v,&rate,sizeof(float));
    return sizeof(float);
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/
/* PacketsTransmitted                                                         */
/* ---------------------------------------------------------------------------*/

size_t metricCalcPacketsTransmitted( MetricValue *mv,   
				     int mnum,
				     void *v, 
				     size_t vlen ) {
  unsigned long long pt = 0;
  unsigned long long p1 = 0;
  unsigned long long p2 = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate PacketsTransmitted\n",
	  __FILE__,__LINE__);
#endif  
  if ( mv && (vlen>=sizeof(unsigned long long)) && (mnum>=1) ) {

    p1 = ip_getPacketsTransmitted(mv[0].mvData);
    if( mnum > 1 ) {
      p2 = ip_getPacketsTransmitted(mv[mnum-1].mvData);
      pt = p1-p2;
    }
    else { pt = p1; }

    //fprintf(stderr,"packets transmitted: %lld\n",pt);
    memcpy(v,&pt,sizeof(unsigned long long));
    return sizeof(unsigned long long);
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/
/* PacketsReceived                                                            */
/* ---------------------------------------------------------------------------*/

size_t metricCalcPacketsReceived( MetricValue *mv,   
				  int mnum,
				  void *v, 
				  size_t vlen ) {
  unsigned long long pr = 0;
  unsigned long long p1 = 0;
  unsigned long long p2 = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate PacketsReceived\n",
	  __FILE__,__LINE__);
#endif

  if ( mv && (vlen>=sizeof(unsigned long long)) && (mnum>=1) ) {

    p1 = ip_getPacketsReceived(mv[0].mvData);
    if( mnum > 1 ) {
      p2 = ip_getPacketsReceived(mv[mnum-1].mvData);
      pr = p1-p2;
    }
    else { pr = p1; }

    //fprintf(stderr,"packets received: %lld\n",pr);
    memcpy(v,&pr,sizeof(unsigned long long));
    return sizeof(unsigned long long);
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/
/* tool functions on BytesSubmitted                                           */
/* ---------------------------------------------------------------------------*/

unsigned long long ip_getBytesReceived( char * data ) {

  char * hlp = NULL;
  char   bytes[128];
  unsigned long long val = 0;

  if( (hlp = strchr(data, ':')) != NULL ) {
    memset(bytes,0,sizeof(bytes));
    strncpy(bytes, data, (strlen(data)-strlen(hlp)) );
    val = atoll(bytes);
  }

  return val;
}


unsigned long long ip_getBytesTransmitted( char * data ) {

  char * hlp = NULL;
  char * end = NULL;
  char   bytes[128];
  unsigned long long val = 0;

  if( (hlp = strchr(data, ':')) != NULL ) {
    hlp++;
    end = strchr(hlp, ':');
    memset(bytes,0,sizeof(bytes));
    strncpy(bytes, hlp, (strlen(hlp)-strlen(end)) );
    val = atoll(bytes);
  }

  return val;
}


unsigned long long ip_getErrorsReceived( char * data ) {
  char * hlp = NULL;
  char * end = NULL;
  char   bytes[128];
  unsigned long long val = 0;

  if( (hlp = strchr(data, ':')) != NULL ) {
    hlp++;
    hlp = strchr(hlp, ':');
    hlp++;
    end = strchr(hlp, ':');
    memset(bytes,0,sizeof(bytes));
    strncpy(bytes, hlp, (strlen(hlp)-strlen(end)) );
    val = atoll(bytes);
  }

  return val;
}


unsigned long long ip_getErrorsTransmitted( char * data ) {
  char * hlp = NULL;
  char * end = NULL;
  char   bytes[128];
  unsigned long long val = 0;

  if( (hlp = strchr(data, ':')) != NULL ) {
    hlp++;
    hlp = strchr(hlp, ':');
    hlp++;
    hlp = strchr(hlp, ':');
    hlp++;
    end = strchr(hlp, ':');
    memset(bytes,0,sizeof(bytes));
    strncpy(bytes, hlp, (strlen(hlp)-strlen(end)) );
    val = atoll(bytes);
  }

  return val;
}


unsigned long long ip_getPacketsReceived( char * data ) {
  char * hlp = NULL;
  char * end = NULL;
  char   bytes[128];
  unsigned long long val = 0;

  if( (hlp = strchr(data, ':')) != NULL ) {
    hlp++;
    hlp = strchr(hlp, ':');
    hlp++;
    hlp = strchr(hlp, ':');
    hlp++;
    hlp = strchr(hlp, ':');
    hlp++;
    end = strchr(hlp, ':');
    memset(bytes,0,sizeof(bytes));
    strncpy(bytes, hlp, (strlen(hlp)-strlen(end)) );
    val = atoll(bytes);
  }

  return val;
}


unsigned long long ip_getPacketsTransmitted( char * data ) {
  char * hlp = NULL;
  char * end = NULL;
  char   bytes[128];
  unsigned long long val = 0;

  if( (hlp = strrchr(data, ':')) != NULL ) {
    hlp++;
    memset(bytes,0,sizeof(bytes));
    strcpy(bytes, hlp);
    val = atoll(bytes);
  }

  return val;
}


/* ---------------------------------------------------------------------------*/
/*                        end of repositoryNetworkPort.c                      */
/* ---------------------------------------------------------------------------*/