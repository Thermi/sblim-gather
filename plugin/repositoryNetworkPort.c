/*
 * $Id: repositoryNetworkPort.c,v 1.1 2004/08/04 09:00:58 heidineu Exp $
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
 * Repository Plugin of the following Network Port specific metrics :
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

/* ---------------------------------------------------------------------------*/

static MetricCalculationDefinition metricCalcDef[4];

/* --- BytesSubmitted is base for :
 * BytesTransmitted, BytesReceived, ErrorRate --- */
static MetricCalculator  metricCalcBytesSubmitted;
static MetricCalculator  metricCalcBytesTransmitted;
static MetricCalculator  metricCalcBytesReceived;
static MetricCalculator  metricCalcErrorRate;

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

  *mcnum=4;
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


/* ---------------------------------------------------------------------------*/
/* BytesReceived                                                              */
/* ---------------------------------------------------------------------------*/

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


/* ---------------------------------------------------------------------------*/
/* ErrorRate                                                                  */
/* ---------------------------------------------------------------------------*/

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
/*                        end of repositoryNetworkPort.c                      */
/* ---------------------------------------------------------------------------*/
