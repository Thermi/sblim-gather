/*
 * $Id: repositoryUnixProcess.c,v 1.1 2004/08/04 09:00:58 heidineu Exp $
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
 * Repository Plugin of the following Unix Process specific metrics :
 *
 * KernelModeTime
 * UserModeTime
 * TotalCPUTime
 * ResidentSetSize
 * PageInCounter
 * PageInRate
 *
 */

/* ---------------------------------------------------------------------------*/

#include <mplugin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------------------------------------------------------------------------*/

static MetricCalculationDefinition metricCalcDef[7];

/* --- CPUTime is base for :
 * KernelModeTime, UserModeTime, TotalCPUTime --- */
static MetricCalculator  metricCalcCPUTime;
static MetricCalculator  metricCalcKernelTime;
static MetricCalculator  metricCalcUserTime;
static MetricCalculator  metricCalcTotalCPUTime;

/* --- ResidentSetSize --- */
static MetricCalculator  metricCalcResSetSize;

/* --- PageInCounter, PageInRate --- */
static MetricCalculator  metricCalcPageInCounter;
static MetricCalculator  metricCalcPageInRate;

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
  metricCalcDef[0].mcName="CPUTime";
  metricCalcDef[0].mcId=mr(pluginname,metricCalcDef[0].mcName);
  metricCalcDef[0].mcMetricType=MD_RETRIEVED|MD_POINT;
  metricCalcDef[0].mcDataType=MD_STRING;
  metricCalcDef[0].mcCalc=metricCalcCPUTime;

  metricCalcDef[1].mcVersion=MD_VERSION;
  metricCalcDef[1].mcName="KernelModeTime";
  metricCalcDef[1].mcId=mr(pluginname,metricCalcDef[1].mcName);
  metricCalcDef[1].mcMetricType=MD_CALCULATED|MD_INTERVAL;
  metricCalcDef[1].mcDataType=MD_UINT64;
  metricCalcDef[1].mcAliasId=metricCalcDef[0].mcId;
  metricCalcDef[1].mcCalc=metricCalcKernelTime;

  metricCalcDef[2].mcVersion=MD_VERSION;
  metricCalcDef[2].mcName="UserModeTime";
  metricCalcDef[2].mcId=mr(pluginname,metricCalcDef[2].mcName);
  metricCalcDef[2].mcMetricType=MD_CALCULATED|MD_INTERVAL;
  metricCalcDef[2].mcDataType=MD_UINT64;
  metricCalcDef[2].mcAliasId=metricCalcDef[0].mcId;
  metricCalcDef[2].mcCalc=metricCalcUserTime;

  metricCalcDef[3].mcVersion=MD_VERSION;
  metricCalcDef[3].mcName="TotalCPUTime";
  metricCalcDef[3].mcId=mr(pluginname,metricCalcDef[3].mcName);
  metricCalcDef[3].mcMetricType=MD_CALCULATED|MD_INTERVAL;
  metricCalcDef[3].mcDataType=MD_UINT64;
  metricCalcDef[3].mcAliasId=metricCalcDef[0].mcId;
  metricCalcDef[3].mcCalc=metricCalcTotalCPUTime;

  metricCalcDef[4].mcVersion=MD_VERSION;
  metricCalcDef[4].mcName="ResidentSetSize";
  metricCalcDef[4].mcId=mr(pluginname,metricCalcDef[4].mcName);
  metricCalcDef[4].mcMetricType=MD_RETRIEVED|MD_POINT;
  metricCalcDef[4].mcDataType=MD_UINT64;
  metricCalcDef[4].mcCalc=metricCalcResSetSize;

  metricCalcDef[5].mcVersion=MD_VERSION;
  metricCalcDef[5].mcName="PageInCounter";
  metricCalcDef[5].mcId=mr(pluginname,metricCalcDef[5].mcName);
  metricCalcDef[5].mcMetricType=MD_RETRIEVED|MD_POINT;
  metricCalcDef[5].mcDataType=MD_UINT64;
  metricCalcDef[5].mcCalc=metricCalcPageInCounter;

  metricCalcDef[6].mcVersion=MD_VERSION;
  metricCalcDef[6].mcName="PageInRate";
  metricCalcDef[6].mcId=mr(pluginname,metricCalcDef[6].mcName);
  metricCalcDef[6].mcMetricType=MD_CALCULATED|MD_RATE;
  metricCalcDef[6].mcDataType=MD_UINT64;
  metricCalcDef[6].mcAliasId=metricCalcDef[5].mcId;
  metricCalcDef[6].mcCalc=metricCalcPageInRate;

  *mcnum=7;
  *mc=metricCalcDef;
  return 0;
}


/* ---------------------------------------------------------------------------*/
/* CPUTime                                                                    */
/* ---------------------------------------------------------------------------*/

/* 
 * The raw data CPUTime has the following syntax :
 *
 * <user mode time>:<kernel mode time>
 *
 * the values in CPUTime are saved in Jiffies ( 1/100ths of a second )
 */

size_t metricCalcCPUTime( MetricValue *mv,   
			  int mnum,
			  void *v, 
			  size_t vlen ) {

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate CPUTime\n",
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
/* KernelModeTime                                                             */
/* ---------------------------------------------------------------------------*/

size_t metricCalcKernelTime( MetricValue *mv,   
			     int mnum,
			     void *v, 
			     size_t vlen ) { 
  char * hlp = NULL;
  char   k_time[sizeof(unsigned long long)+1];
  unsigned long long kt = 0;
  unsigned long long k1 = 0;
  unsigned long long k2 = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate KernelModeTime\n",
	  __FILE__,__LINE__);
#endif
  /* 
   * the KernelModeTime is based on the second entry of the CPUTime 
   * value and needs to be multiplied by 10 to get milliseconds
   *
   */

  /*
  fprintf(stderr,"mnum : %i\n",mnum);
  fprintf(stderr,"mv[0].mvData : %s\n",mv[0].mvData);
  fprintf(stderr,"mv[mnum-1].mvData : %s\n",mv[mnum-1].mvData);
  fprintf(stderr,"vlen : %i\n",vlen);
  fprintf(stderr,"mv->mvDataLength : %i\n",mv->mvDataLength);
  fprintf(stderr,"sizeof(unsigned long long) : %i\n",sizeof(unsigned long long));  
  */

  if ( mv && (vlen>=sizeof(unsigned long long)) && (mnum>=1) ) {
    hlp = strchr(mv[0].mvData, ':');
    hlp++;
    memset(k_time,0,sizeof(k_time));
    strncpy(k_time, hlp, strlen(hlp) );
    k1 = atoll(k_time)*10;
      
    if( mnum > 1 ) {
      hlp = strchr(mv[mnum-1].mvData, ':');
      hlp++;
      memset(k_time,0,sizeof(k_time));
      strncpy(k_time, hlp, strlen(hlp) );
      k2 = atoll(k_time)*10;
      
      kt = (k1-k2)/2;
    }
    else { kt = k1; }
    
    //fprintf(stderr,"kernel time: %lld\n",kt);
    memcpy(v,&kt,sizeof(unsigned long long));
    return sizeof(unsigned long long);
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/
/* UserModeTime                                                               */
/* ---------------------------------------------------------------------------*/

size_t metricCalcUserTime( MetricValue *mv,   
			   int mnum,
			   void *v, 
			   size_t vlen ) { 
  char * hlp = NULL;
  char * end = NULL;
  char   u_time[sizeof(unsigned long long)+1];
  unsigned long long ut = 0;
  unsigned long long u1 = 0;
  unsigned long long u2 = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate UserModeTime\n",
	  __FILE__,__LINE__);
#endif
  /* 
   * the UserModeTime is based on the first entry of the CPUTime 
   * value and needs to be multiplied by 10 to get milliseconds
   *
   */

  /*
  fprintf(stderr,"mnum : %i\n",mnum);
  fprintf(stderr,"mv[0].mvData : %s\n",mv[0].mvData);
  fprintf(stderr,"mv[mnum-1].mvData : %s\n",mv[mnum-1].mvData);
  fprintf(stderr,"vlen : %i\n",vlen);
  fprintf(stderr,"mv->mvDataLength : %i\n",mv->mvDataLength);
  fprintf(stderr,"sizeof(unsigned long long) : %i\n",sizeof(unsigned long long));  
  */

  if ( mv && (vlen>=sizeof(unsigned long long)) && (mnum>=1) ) {
    hlp = mv[0].mvData;
    end = strchr(hlp, ':');
    memset(u_time,0,sizeof(u_time));
    strncpy(u_time, hlp, (strlen(hlp)-strlen(end)) );
    u1 = atoll(u_time)*10;
    
    if( mnum > 1 ) {
      hlp = mv[mnum-1].mvData;
      end = strchr(hlp, ':');
      memset(u_time,0,sizeof(u_time));
      strncpy(u_time, hlp, (strlen(hlp)-strlen(end)) );
      u2 = atoll(u_time)*10;
      
      ut = (u1-u2)/2;
    }
    else { ut = u1; }
    
    //fprintf(stderr,"user time: %lld\n",ut);
    memcpy(v,&ut,sizeof(unsigned long long));
    return sizeof(unsigned long long);
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/
/* TotalCPUTime                                                               */
/* ---------------------------------------------------------------------------*/

size_t metricCalcTotalCPUTime( MetricValue *mv,   
			       int mnum,
			       void *v, 
			       size_t vlen ) { 
  char * hlp = NULL;
  char * end = NULL;
  char   u_time[sizeof(unsigned long long)+1];
  char   k_time[sizeof(unsigned long long)+1];
  unsigned long long u1 = 0;
  unsigned long long u2 = 0;
  unsigned long long k1 = 0;
  unsigned long long k2 = 0;
  unsigned long long total = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate TotalCPUTime\n",
	  __FILE__,__LINE__);
#endif
  /* 
   * the TotalCPUTime is based on both entries of the CPUTime 
   * value and needs to be multiplied by 10 to get milliseconds
   *
   */

  /*
  fprintf(stderr,"mnum : %i\n",mnum);
  fprintf(stderr,"mv[0].mvData : %s\n",mv[0].mvData);
  fprintf(stderr,"mv[mnum-1].mvData : %s\n",mv[mnum-1].mvData);
  fprintf(stderr,"vlen : %i\n",vlen);
  fprintf(stderr,"mv->mvDataLength : %i\n",mv->mvDataLength);
  fprintf(stderr,"sizeof(unsigned long long) : %i\n",sizeof(unsigned long long));  
  */

  if ( mv && (vlen>=sizeof(unsigned long long)) && (mnum>=1) ) {
    hlp = mv[0].mvData;
    end = strchr(hlp, ':');
    memset(u_time,0,sizeof(u_time));
    strncpy(u_time, hlp, (strlen(hlp)-strlen(end)) );
    u1 = atoll(u_time)*10;

    hlp = end+1;
    memset(k_time,0,sizeof(k_time));
    strncpy(k_time, hlp, strlen(hlp) );
    k1 = atoll(k_time)*10;

    if( mnum > 1 ) {
      hlp = mv[mnum-1].mvData;
      end = strchr(hlp, ':');
      memset(u_time,0,sizeof(u_time));
      strncpy(u_time, hlp, (strlen(hlp)-strlen(end)) );
      u2 = atoll(u_time)*10;

      hlp = end+1;
      memset(k_time,0,sizeof(k_time));
      strncpy(k_time, hlp, strlen(hlp) );
      k2 = atoll(k_time)*10;
      
      total = ((u1-u2)+(k1-k2))/2;
    }
    else { total = u1+k1; }

    //fprintf(stderr,"total time: %lld\n",total);
    memcpy(v,&total,sizeof(unsigned long long));
    return sizeof(unsigned long long);
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/
/* ResidentSetSize                                                            */
/* ---------------------------------------------------------------------------*/

size_t metricCalcResSetSize( MetricValue *mv,   
			     int mnum,
			     void *v, 
			     size_t vlen ) {

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate ResidentSetSize\n",
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
/* PageInCounter                                                              */
/* ---------------------------------------------------------------------------*/

size_t metricCalcPageInCounter( MetricValue *mv,   
				int mnum,
				void *v, 
				size_t vlen ) {

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate PageInCounter\n",
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
/* PageInRate                                                                 */
/* ---------------------------------------------------------------------------*/

size_t metricCalcPageInRate( MetricValue *mv,   
			     int mnum,
			     void *v, 
			     size_t vlen ) {
  unsigned long long total = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate PageInRate\n",
	  __FILE__,__LINE__);
#endif
  if ( mv && (vlen>=sizeof(unsigned long long)) && (mnum>=2) ) {
    total = (*(unsigned long long*)mv[0].mvData - *(unsigned long long*)mv[mnum-1].mvData) / 
            (mv[0].mvTimeStamp - mv[mnum-1].mvTimeStamp);
    memcpy(v, &total, sizeof(unsigned long long));
    return sizeof(unsigned long long);
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/
/*                       end of repositoryUnixProcess.c                       */
/* ---------------------------------------------------------------------------*/
