/*
 * $Id: repositoryOperatingSystem.c,v 1.2 2004/08/03 11:24:46 heidineu Exp $
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
 * Repository Plugin of the following Operating System specific metrics :
 * 
 * NumberOfUsers
 * NumberOfProcesses
 * CPUTime
 * KernelModeTime
 * UserModeTime
 * TotalCPUTime
 * MemorySize
 * TotalVisibleMemorySize
 * FreePhysicalMemory
 * SizeStoredInPagingFiles
 * FreeSpaceInPagingFiles
 * TotalVirtualMemorySize
 * FreeVirtualMemory
 * PageInCounter
 * PageInRate
 * LoadCounter
 * LoadAverage
 *
 */

/* ---------------------------------------------------------------------------*/

#include <mplugin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     
#include <time.h>

#include <unistd.h>
#include <sys/types.h>

/* ---------------------------------------------------------------------------*/

static MetricCalculationDefinition metricCalcDef[17];

/* --- NumberOfUsers --- */
static MetricCalculator  metricCalcNumOfUser;

/* --- NumberOfProcesses --- */
static MetricCalculator  metricCalcNumOfProc;

/* --- CPUTime is base for :
 * KernelModeTime, UserModeTime, TotalCPUTime --- */
static MetricCalculator  metricCalcCPUTime;
static MetricCalculator  metricCalcKernelTime;
static MetricCalculator  metricCalcUserTime;
static MetricCalculator  metricCalcTotalCPUTime;

/* --- MemorySize is base for :
 * TotalVisibleMemorySize,  FreePhysicalMemory, 
 * SizeStoredInPagingFiles, FreeSpaceInPagingFiles,
 * TotalVirtualMemorySize,  FreeVirtualMemory --- */
static MetricCalculator  metricCalcMemorySize;
static MetricCalculator  metricCalcTotalPhysMem;
static MetricCalculator  metricCalcFreePhysMem;
static MetricCalculator  metricCalcTotalSwapMem;
static MetricCalculator  metricCalcFreeSwapMem;
static MetricCalculator  metricCalcTotalVirtMem;
static MetricCalculator  metricCalcFreeVirtMem;

/* --- PageInCounter, PageInRate --- */
static MetricCalculator  metricCalcPageInCounter;
static MetricCalculator  metricCalcPageInRate;

/* --- LoadCounter, LoadAverage --- */
static MetricCalculator  metricCalcLoadCounter;
static MetricCalculator  metricCalcLoadAverage;

/* ---------------------------------------------------------------------------*/

int _DefinedRepositoryMetrics (MetricRegisterId *mr,
			       const char *pluginname,
			       size_t *mcnum,
			       MetricCalculationDefinition **mc) {
#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Retrieving metric calculation definitions\n",
	  __FILE__,__LINE__);
#endif
  if (mr==NULL||mcnum==NULL||mc==NULL) {
    fprintf(stderr,"--- %s(%i) : invalid parameter list\n",__FILE__,__LINE__);
    return -1;
  }

  metricCalcDef[0].mcVersion=MD_VERSION;
  metricCalcDef[0].mcName="NumberOfUsers";
  metricCalcDef[0].mcId=mr(pluginname,metricCalcDef[0].mcName);
  metricCalcDef[0].mcMetricType=MD_RETRIEVED|MD_POINT;
  metricCalcDef[0].mcDataType=MD_UINT32;
  metricCalcDef[0].mcCalc=metricCalcNumOfUser;

  metricCalcDef[1].mcVersion=MD_VERSION;
  metricCalcDef[1].mcName="NumberOfProcesses";
  metricCalcDef[1].mcId=mr(pluginname,metricCalcDef[1].mcName);
  metricCalcDef[1].mcMetricType=MD_RETRIEVED|MD_POINT;
  metricCalcDef[1].mcDataType=MD_UINT32;
  metricCalcDef[1].mcCalc=metricCalcNumOfProc;

  metricCalcDef[2].mcVersion=MD_VERSION;
  metricCalcDef[2].mcName="CPUTime";
  metricCalcDef[2].mcId=mr(pluginname,metricCalcDef[2].mcName);
  metricCalcDef[2].mcMetricType=MD_RETRIEVED|MD_POINT;
  metricCalcDef[2].mcDataType=MD_STRING;
  metricCalcDef[2].mcCalc=metricCalcCPUTime;

  metricCalcDef[3].mcVersion=MD_VERSION;
  metricCalcDef[3].mcName="KernelModeTime";
  metricCalcDef[3].mcId=mr(pluginname,metricCalcDef[3].mcName);
  metricCalcDef[3].mcMetricType=MD_CALCULATED|MD_INTERVAL;
  metricCalcDef[3].mcDataType=MD_UINT64;
  metricCalcDef[3].mcAliasId=metricCalcDef[2].mcId;
  metricCalcDef[3].mcCalc=metricCalcKernelTime;

  metricCalcDef[4].mcVersion=MD_VERSION;
  metricCalcDef[4].mcName="UserModeTime";
  metricCalcDef[4].mcId=mr(pluginname,metricCalcDef[4].mcName);
  metricCalcDef[4].mcMetricType=MD_CALCULATED|MD_INTERVAL;
  metricCalcDef[4].mcDataType=MD_UINT64;
  metricCalcDef[4].mcAliasId=metricCalcDef[2].mcId;
  metricCalcDef[4].mcCalc=metricCalcUserTime;

  metricCalcDef[5].mcVersion=MD_VERSION;
  metricCalcDef[5].mcName="TotalCPUTime";
  metricCalcDef[5].mcId=mr(pluginname,metricCalcDef[5].mcName);
  metricCalcDef[5].mcMetricType=MD_CALCULATED|MD_INTERVAL;
  metricCalcDef[5].mcDataType=MD_UINT64;
  metricCalcDef[5].mcAliasId=metricCalcDef[2].mcId;
  metricCalcDef[5].mcCalc=metricCalcTotalCPUTime;

  metricCalcDef[6].mcVersion=MD_VERSION;
  metricCalcDef[6].mcName="MemorySize";
  metricCalcDef[6].mcId=mr(pluginname,metricCalcDef[6].mcName);
  metricCalcDef[6].mcMetricType=MD_RETRIEVED|MD_POINT;
  metricCalcDef[6].mcDataType=MD_STRING;
  metricCalcDef[6].mcCalc=metricCalcMemorySize;

  metricCalcDef[7].mcVersion=MD_VERSION;
  metricCalcDef[7].mcName="TotalVisibleMemorySize";
  metricCalcDef[7].mcId=mr(pluginname,metricCalcDef[7].mcName);
  metricCalcDef[7].mcMetricType=MD_CALCULATED|MD_POINT;
  metricCalcDef[7].mcDataType=MD_UINT64;
  metricCalcDef[7].mcAliasId=metricCalcDef[6].mcId;
  metricCalcDef[7].mcCalc=metricCalcTotalPhysMem;

  metricCalcDef[8].mcVersion=MD_VERSION;
  metricCalcDef[8].mcName="FreePhysicalMemory";
  metricCalcDef[8].mcId=mr(pluginname,metricCalcDef[8].mcName);
  metricCalcDef[8].mcMetricType=MD_CALCULATED|MD_POINT;
  metricCalcDef[8].mcDataType=MD_UINT64;
  metricCalcDef[8].mcAliasId=metricCalcDef[6].mcId;
  metricCalcDef[8].mcCalc=metricCalcFreePhysMem;

  metricCalcDef[9].mcVersion=MD_VERSION;
  metricCalcDef[9].mcName="SizeStoredInPagingFiles";
  metricCalcDef[9].mcId=mr(pluginname,metricCalcDef[9].mcName);
  metricCalcDef[9].mcMetricType=MD_CALCULATED|MD_POINT;
  metricCalcDef[9].mcDataType=MD_UINT64;
  metricCalcDef[9].mcAliasId=metricCalcDef[6].mcId;
  metricCalcDef[9].mcCalc=metricCalcTotalSwapMem;

  metricCalcDef[10].mcVersion=MD_VERSION;
  metricCalcDef[10].mcName="FreeSpaceInPagingFiles";
  metricCalcDef[10].mcId=mr(pluginname,metricCalcDef[10].mcName);
  metricCalcDef[10].mcMetricType=MD_CALCULATED|MD_POINT;
  metricCalcDef[10].mcDataType=MD_UINT64;
  metricCalcDef[10].mcAliasId=metricCalcDef[6].mcId;
  metricCalcDef[10].mcCalc=metricCalcFreeSwapMem;

  metricCalcDef[11].mcVersion=MD_VERSION;
  metricCalcDef[11].mcName="TotalVirtualMemorySize";
  metricCalcDef[11].mcId=mr(pluginname,metricCalcDef[11].mcName);
  metricCalcDef[11].mcMetricType=MD_CALCULATED|MD_POINT;
  metricCalcDef[11].mcDataType=MD_UINT64;
  metricCalcDef[11].mcAliasId=metricCalcDef[6].mcId;
  metricCalcDef[11].mcCalc=metricCalcTotalVirtMem;

  metricCalcDef[12].mcVersion=MD_VERSION;
  metricCalcDef[12].mcName="FreeVirtualMemory";
  metricCalcDef[12].mcId=mr(pluginname,metricCalcDef[12].mcName);
  metricCalcDef[12].mcMetricType=MD_CALCULATED|MD_POINT;
  metricCalcDef[12].mcDataType=MD_UINT64;
  metricCalcDef[12].mcAliasId=metricCalcDef[6].mcId;
  metricCalcDef[12].mcCalc=metricCalcFreeVirtMem;

  metricCalcDef[13].mcVersion=MD_VERSION;
  metricCalcDef[13].mcName="PageInCounter";
  metricCalcDef[13].mcId=mr(pluginname,metricCalcDef[13].mcName);
  metricCalcDef[13].mcMetricType=MD_RETRIEVED|MD_POINT;
  metricCalcDef[13].mcDataType=MD_UINT64;
  metricCalcDef[13].mcCalc=metricCalcPageInCounter;

  metricCalcDef[14].mcVersion=MD_VERSION;
  metricCalcDef[14].mcName="PageInRate";
  metricCalcDef[14].mcId=mr(pluginname,metricCalcDef[14].mcName);
  metricCalcDef[14].mcMetricType=MD_CALCULATED|MD_RATE;
  metricCalcDef[14].mcDataType=MD_UINT64;
  metricCalcDef[14].mcAliasId=metricCalcDef[13].mcId;
  metricCalcDef[14].mcCalc=metricCalcPageInRate;

  metricCalcDef[15].mcVersion=MD_VERSION;
  metricCalcDef[15].mcName="LoadCounter";
  metricCalcDef[15].mcId=mr(pluginname,metricCalcDef[15].mcName);
  metricCalcDef[15].mcMetricType=MD_RETRIEVED|MD_POINT;
  metricCalcDef[15].mcDataType=MD_FLOAT32;
  metricCalcDef[15].mcCalc=metricCalcLoadCounter;

  metricCalcDef[16].mcVersion=MD_VERSION;
  metricCalcDef[16].mcName="LoadAverage";
  metricCalcDef[16].mcId=mr(pluginname,metricCalcDef[16].mcName);
  metricCalcDef[16].mcMetricType=MD_CALCULATED|MD_AVERAGE;
  metricCalcDef[16].mcDataType=MD_FLOAT32;
  metricCalcDef[16].mcAliasId=metricCalcDef[15].mcId;
  metricCalcDef[16].mcCalc=metricCalcLoadAverage;

  *mcnum=17;
  *mc=metricCalcDef;
  return 0;
}


/* ---------------------------------------------------------------------------*/
/* NumberOfUsers                                                              */
/* ---------------------------------------------------------------------------*/

size_t metricCalcNumOfUser( MetricValue *mv,  
			    int mnum,
			    void *v, 
			    size_t vlen ) {

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate NumberOfUsers\n",
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
/* NumberOfProcesses                                                          */
/* ---------------------------------------------------------------------------*/

size_t metricCalcNumOfProc( MetricValue *mv,   
			    int mnum,
			    void *v, 
			    size_t vlen ) {

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate NumberOfProcesses\n",
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
/* CPUTime                                                                    */
/* ---------------------------------------------------------------------------*/

/* 
 * The raw data CPUTime has the following syntax :
 *
 * <user mode>:<user mode with low priority(nice)>:<system mode>:<idle task>
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
  char * end = NULL;
  char   k_time[sizeof(unsigned long long)+1];
  int    i   = 0;
  unsigned long long kt = 0;
  unsigned long long k1 = 0;
  unsigned long long k2 = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate KernelModeTime\n",
	  __FILE__,__LINE__);
#endif
  /* 
   * the KernelModeTime is based on the third entry of the CPUTime 
   * value and needs to be multiplied by 10 to get microseconds
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
    for( i=0; i<2; i++ ) { 
      hlp = strchr(hlp, ':');
      hlp++;
    }
    end = strchr(hlp, ':');
    memset(k_time,0,sizeof(k_time));
    strncpy(k_time, hlp, (strlen(hlp)-strlen(end)) );
    k1 = atoll(k_time)*10;

    if( mnum > 1 ) {
      hlp = mv[mnum-1].mvData;
      for( i=0; i<2; i++ ) { 
	hlp = strchr(hlp, ':');
	hlp++;
      }
      end = strchr(hlp, ':');
      memset(k_time,0,sizeof(k_time));
      strncpy(k_time, hlp, (strlen(hlp)-strlen(end)) );
      k2 = atoll(k_time)*10;
      
      kt = (k1-k2)/2;
    }
    else { kt = k1; }

    fprintf(stderr,"kernel time: %lld\n",kt);
    
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
   * value and needs to be multiplied by 10 to get microseconds
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
    /*
    fprintf(stderr,"u_time : %s\n",u_time);
    fprintf(stderr,"u1 : %lld\n",u1);
    */

    if( mnum > 1 ) {
      hlp = mv[mnum-1].mvData;
      end = strchr(hlp, ':');
      memset(u_time,0,sizeof(u_time));
      strncpy(u_time, hlp, (strlen(hlp)-strlen(end)) );
      u2 = atoll(u_time)*10;
      /*
      fprintf(stderr,"u_time : %s\n",u_time);
      fprintf(stderr,"u2 : %lld\n",u2);
      */
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
  char   i_time[sizeof(unsigned long long)+1];
  int    i   = 0;
  unsigned long long u1 = 0;
  unsigned long long u2 = 0;
  unsigned long long k1 = 0;
  unsigned long long k2 = 0;
  unsigned long long i1 = 0;
  unsigned long long i2 = 0;
  unsigned long long total = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate TotalCPUTime\n",
	  __FILE__,__LINE__);
#endif
  /* 
   * the UserModeTime is based on the first, third and last entry
   * of the CPUTime value and needs to be multiplied by 10 to get
   * microseconds
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

    for( i=0; i<2; i++ ) { 
      hlp = strchr(hlp, ':');
      hlp++;
    }
    end = strchr(hlp, ':');
    memset(k_time,0,sizeof(k_time));
    strncpy(k_time, hlp, (strlen(hlp)-strlen(end)) );
    k1 = atoll(k_time)*10;

    hlp = end+1;
    memset(i_time,0,sizeof(i_time));
    strncpy(i_time, hlp, strlen(hlp) );
    i1 = atoll(i_time)*10;


    if( mnum > 1 ) {
      hlp = mv[mnum-1].mvData;
      end = strchr(hlp, ':');
      memset(u_time,0,sizeof(u_time));
      strncpy(u_time, hlp, (strlen(hlp)-strlen(end)) );
      u2 = atoll(u_time)*10;

      for( i=0; i<2; i++ ) { 
	hlp = strchr(hlp, ':');
	hlp++;
      }
      end = strchr(hlp, ':');
      memset(k_time,0,sizeof(k_time));
      strncpy(k_time, hlp, (strlen(hlp)-strlen(end)) );
      k2 = atoll(k_time)*10;

      hlp = end+1;
      memset(i_time,0,sizeof(i_time));
      strncpy(i_time, hlp, strlen(hlp) );
      i2 = atoll(i_time)*10;
      
      total = ((u1-u2)+(k1-k2)+(i1-i2))/2;
    }
    else { total = u1+k1+i1; }
    
    //fprintf(stderr,"total time: %lld\n",total);
    memcpy(v,&total,sizeof(unsigned long long));
    return sizeof(unsigned long long);
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/
/* MemorySize                                                                 */
/* ---------------------------------------------------------------------------*/

/* 
 * The raw data MemorySize has the following syntax :
 *
 * <TotalVisibleMemorySize>:<FreePhysicalMemory>:<SizeStoredInPagingFiles>:<FreeSpaceInPagingFiles>
 *
 * the values in MemorySize are saved in kBytes
 */

size_t metricCalcMemorySize( MetricValue *mv,   
			     int mnum,
			     void *v, 
			     size_t vlen ) {

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate MemorySize\n",
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
/* TotalVisibleMemorySize                                                     */
/* ---------------------------------------------------------------------------*/

size_t metricCalcTotalPhysMem( MetricValue *mv,   
			       int mnum,
			       void *v, 
			       size_t vlen ) { 
  char             * hlp  = NULL;
  char             * end  = NULL;
  char             * pmem = NULL;
  unsigned long long mem  = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate TotalVisibleMemorySize\n",
	  __FILE__,__LINE__);
#endif
  /* 
   * the TotalVisibleMemorySize is based on the first entry of the 
   * MemorySize value
   *
   */

  if ( mv && (vlen>=sizeof(unsigned long long)) && (mnum==1) ) {

    hlp = mv->mvData;
    end = strchr(hlp, ':');

    pmem = calloc(1, (strlen(hlp)-strlen(end)+1) );
    strncpy(pmem, hlp, (strlen(hlp)-strlen(end)) );
    mem = atoll(pmem);
    free(pmem);

    memcpy(v,&mem,sizeof(unsigned long long));
    return sizeof(unsigned long long);
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/
/* FreePhysicalMemory                                                         */
/* ---------------------------------------------------------------------------*/

size_t metricCalcFreePhysMem( MetricValue *mv,   
			      int mnum,
			      void *v, 
			      size_t vlen ) { 
  char             * hlp  = NULL;
  char             * end  = NULL;
  char             * pmem = NULL;
  unsigned long long mem  = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate FreePhysicalMemory\n",
	  __FILE__,__LINE__);
#endif
  /* 
   * the FreePhysicalMemory is based on the second entry of the 
   * MemorySize value
   *
   */

  if ( mv && (vlen>=sizeof(unsigned long long)) && (mnum==1) ) {

    hlp = strchr(mv->mvData, ':');
    hlp++;
    end = strchr(hlp, ':');

    pmem = calloc(1, (strlen(hlp)-strlen(end)+1) );
    strncpy(pmem, hlp, (strlen(hlp)-strlen(end)) );
    mem = atoll(pmem);
    free(pmem);

    memcpy(v,&mem,sizeof(unsigned long long));
    return sizeof(unsigned long long);
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/
/* SizeStoredInPagingFiles                                                    */
/* ---------------------------------------------------------------------------*/

size_t metricCalcTotalSwapMem( MetricValue *mv,   
			       int mnum,
			       void *v, 
			       size_t vlen ) { 
  char             * hlp  = NULL;
  char             * end  = NULL;
  char             * pmem = NULL;
  unsigned long long mem  = 0;
  int                i    = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate SizeStoredInPagingFiles\n",
	  __FILE__,__LINE__);
#endif
  /* 
   * the SizeStoredInPagingFiles is based on the third entry of the 
   * MemorySize value
   *
   */

  if ( mv && (vlen>=sizeof(unsigned long long)) && (mnum==1) ) {

    hlp = mv->mvData;
    for(;i<2;i++) {
      hlp = strchr(hlp, ':');
      hlp++;
    }
    end = strchr(hlp, ':');

    pmem = calloc(1, (strlen(hlp)-strlen(end)+1) );
    strncpy(pmem, hlp, (strlen(hlp)-strlen(end)) );
    mem = atoll(pmem);
    free(pmem);

    memcpy(v,&mem,sizeof(unsigned long long));
    return sizeof(unsigned long long);
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/
/* FreeSpaceInPagingFiles                                                     */
/* ---------------------------------------------------------------------------*/

size_t metricCalcFreeSwapMem( MetricValue *mv,   
			      int mnum,
			      void *v, 
			      size_t vlen ) { 
  char             * hlp  = NULL;
  char             * pmem = NULL;
  unsigned long long mem  = 0;
  int                i    = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate FreeSpaceInPagingFiles\n",
	  __FILE__,__LINE__);
#endif
  /* 
   * the FreeSpaceInPagingFiles is based on the last entry of the 
   * MemorySize value
   *
   */

  if ( mv && (vlen>=sizeof(unsigned long long)) && (mnum==1) ) {

    hlp = mv->mvData;
    for(;i<3;i++) {
      hlp = strchr(hlp, ':');
      hlp++;
    }

    pmem = calloc(1, (strlen(hlp)+1) );
    strcpy(pmem, hlp );
    mem = atoll(pmem);
    free(pmem);

    memcpy(v,&mem,sizeof(unsigned long long));
    return sizeof(unsigned long long);
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/
/* TotalVirtualMemorySize                                                     */
/* ---------------------------------------------------------------------------*/

size_t metricCalcTotalVirtMem( MetricValue *mv,  
			       int mnum, 
			       void *v, 
			       size_t vlen ) { 
  char             * hlp  = NULL;
  char             * end  = NULL;
  char             * pmem = NULL;
  char             * smem = NULL;
  unsigned long long mem  = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate TotalVirtualMemorySize\n",
	  __FILE__,__LINE__);
#endif
  /* 
   * the TotalVirtualMemorySize is based on the first and the third 
   * entry of the MemorySize value
   *
   */

  if ( mv && (vlen>=sizeof(unsigned long long)) && (mnum==1) ) {

    hlp = mv->mvData;
    end = strchr(hlp, ':');
    pmem = calloc(1, (strlen(hlp)-strlen(end)+1) );
    strncpy(pmem, hlp, (strlen(hlp)-strlen(end)) );

    hlp = strchr(end+1, ':');
    hlp++;
    end = strchr(hlp, ':');
    smem = calloc(1, (strlen(hlp)-strlen(end)+1) );
    strncpy(smem, hlp, (strlen(hlp)-strlen(end)) );

    mem = atoll(pmem)+atoll(smem);
    free(pmem);
    free(smem);

    memcpy(v,&mem,sizeof(unsigned long long));
    return sizeof(unsigned long long);
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/
/*  FreeVirtualMemory                                                         */
/* ---------------------------------------------------------------------------*/

size_t metricCalcFreeVirtMem( MetricValue *mv,   
			      int mnum,
			      void *v, 
			      size_t vlen ) { 
  char             * hlp  = NULL;
  char             * end  = NULL;
  char             * pmem = NULL;
  char             * smem = NULL;
  unsigned long long mem  = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate FreeVirtualMemory\n",
	  __FILE__,__LINE__);
#endif
  /* 
   * the FreeVirtualMemory is based on the second and the last entry
   * of the MemorySize value
   *
   */

  if ( mv && (vlen>=sizeof(unsigned long long)) && (mnum==1) ) {

    hlp = strchr(mv->mvData, ':');
    hlp++;
    end = strchr(hlp, ':');
    pmem = calloc(1, (strlen(hlp)-strlen(end)+1) );
    strncpy(pmem, hlp, (strlen(hlp)-strlen(end)) );

    hlp = strchr(end+1, ':');
    hlp++;
    smem = calloc(1, (strlen(hlp)+1) );
    strcpy(smem, hlp );

    mem = atoll(pmem)+atoll(smem);
    free(pmem);
    free(smem);

    memcpy(v,&mem,sizeof(unsigned long long));
    return sizeof(unsigned long long);
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
    total = (*(unsigned long long*)mv[0].mvData - *(unsigned long long*)mv[mnum-1].mvData) / (mv[0].mvTimeStamp - mv[mnum-1].mvTimeStamp);
    memcpy(v, &total, sizeof(unsigned long long));
    return sizeof(unsigned long long);
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/
/* LoadCounter                                                                */
/* ---------------------------------------------------------------------------*/

size_t metricCalcLoadCounter( MetricValue *mv,   
			      int mnum,
			      void *v, 
			      size_t vlen ) {

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate LoadCounter\n",
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
/* LoadAverage                                                                */
/* ---------------------------------------------------------------------------*/

size_t metricCalcLoadAverage( MetricValue *mv,   
			      int mnum,
			      void *v, 
			      size_t vlen ) {
  int   i     = 0;
  float total = 0;
  float sum   = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate LoadAverage\n",
	  __FILE__,__LINE__);
#endif
  if ( mv && (vlen>=sizeof(float)) && (mnum>=2) ) {
    for(;i<mnum;i++) { sum = sum + *(float*)mv[i].mvData; }
    total = sum / mnum;
    memcpy(v, &total, sizeof(float));
    return sizeof(float);
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/
/*                    end of metricOperatingSystem.c                          */
/* ---------------------------------------------------------------------------*/
