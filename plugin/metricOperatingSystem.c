/*
 * $Id: metricOperatingSystem.c,v 1.1 2003/10/17 13:56:01 mihajlov Exp $
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
 * following Operating System specific metrics :
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
#include <dirent.h>

/* ---------------------------------------------------------------------------*/

static MetricDefinition  metricDef[17];

static ResourceLister    resourceLister;
static ResourceListDeallocator  resourceListDeallocator;

/* --- NumberOfUsers --- */
static MetricRetriever   metricRetrNumOfUser;
static MetricCalculator  metricCalcNumOfUser;

/* --- NumberOfProcesses --- */
static MetricRetriever   metricRetrNumOfProc;
static MetricCalculator  metricCalcNumOfProc;

/* --- CPUTime is base for :
 * KernelModeTime, UserModeTime, TotalCPUTime --- */
static MetricRetriever   metricRetrCPUTime;
static MetricCalculator  metricCalcCPUTime;

static MetricCalculator  metricCalcKernelTime;
static MetricCalculator  metricCalcUserTime;
static MetricCalculator  metricCalcTotalCPUTime;

/* --- MemorySize is base for :
 * TotalVisibleMemorySize,  FreePhysicalMemory, 
 * SizeStoredInPagingFiles, FreeSpaceInPagingFiles,
 * TotalVirtualMemorySize,  FreeVirtualMemory --- */
static MetricRetriever   metricRetrMemorySize;
static MetricCalculator  metricCalcMemorySize;

static MetricCalculator  metricCalcTotalPhysMem;
static MetricCalculator  metricCalcFreePhysMem;
static MetricCalculator  metricCalcTotalSwapMem;
static MetricCalculator  metricCalcFreeSwapMem;
static MetricCalculator  metricCalcTotalVirtMem;
static MetricCalculator  metricCalcFreeVirtMem;

/* --- PageInCounter, PageInRate --- */
static MetricRetriever   metricRetrPageInCounter;
static MetricCalculator  metricCalcPageInCounter;

static MetricCalculator  metricCalcPageInRate;

/* --- LoadCounter, LoadAverage --- */
static MetricRetriever   metricRetrLoadCounter;
static MetricCalculator  metricCalcLoadCounter;

static MetricCalculator  metricCalcLoadAverage;

/* ---------------------------------------------------------------------------*/

static char * resourceList[1] = { "OperatingSystem" };

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

  metricDef[0].mdName="NumberOfUsers";
  metricDef[0].mdId=mr(pluginname,metricDef[0].mdName);
  metricDef[0].mdSampleInterval=60;
  metricDef[0].mdMetricType=MD_RETRIEVED|MD_POINT;
  metricDef[0].mdDataType=MD_UINT32;
  metricDef[0].mproc=metricRetrNumOfUser;
  metricDef[0].mdeal=free;
  metricDef[0].mcalc=metricCalcNumOfUser;
  metricDef[0].mresl=resourceLister;
  metricDef[0].mresldeal=resourceListDeallocator;

  metricDef[1].mdName="NumberOfProcesses";
  metricDef[1].mdId=mr(pluginname,metricDef[1].mdName);
  metricDef[1].mdSampleInterval=60;
  metricDef[1].mdMetricType=MD_RETRIEVED|MD_POINT;
  metricDef[1].mdDataType=MD_UINT32;
  metricDef[1].mproc=metricRetrNumOfProc;
  metricDef[1].mdeal=free;
  metricDef[1].mcalc=metricCalcNumOfProc;
  metricDef[1].mresl=metricDef[0].mresl;
  metricDef[1].mresldeal=metricDef[0].mresldeal;

  metricDef[2].mdName="CPUTime";
  metricDef[2].mdId=mr(pluginname,metricDef[2].mdName);
  metricDef[2].mdSampleInterval=60;
  metricDef[2].mdMetricType=MD_RETRIEVED|MD_POINT;
  metricDef[2].mdDataType=MD_STRING;
  metricDef[2].mproc=metricRetrCPUTime;
  metricDef[2].mdeal=free;
  metricDef[2].mcalc=metricCalcCPUTime;
  metricDef[2].mresl=metricDef[0].mresl;
  metricDef[2].mresldeal=metricDef[0].mresldeal;

  metricDef[3].mdName="KernelModeTime";
  metricDef[3].mdId=mr(pluginname,metricDef[3].mdName);
  metricDef[3].mdMetricType=MD_CALCULATED|MD_INTERVAL;
  metricDef[3].mdDataType=MD_UINT64;
  metricDef[3].mdAliasId=metricDef[2].mdId;
  metricDef[3].mproc=NULL;
  metricDef[3].mdeal=NULL;
  metricDef[3].mcalc=metricCalcKernelTime;
  metricDef[3].mresl=metricDef[0].mresl;
  metricDef[3].mresldeal=metricDef[0].mresldeal;

  metricDef[4].mdName="UserModeTime";
  metricDef[4].mdId=mr(pluginname,metricDef[4].mdName);
  metricDef[4].mdMetricType=MD_CALCULATED|MD_INTERVAL;
  metricDef[4].mdDataType=MD_UINT64;
  metricDef[4].mdAliasId=metricDef[2].mdId;
  metricDef[4].mproc=NULL;
  metricDef[4].mdeal=NULL;
  metricDef[4].mcalc=metricCalcUserTime;
  metricDef[4].mresl=metricDef[0].mresl;
  metricDef[4].mresldeal=metricDef[0].mresldeal;

  metricDef[5].mdName="TotalCPUTime";
  metricDef[5].mdId=mr(pluginname,metricDef[5].mdName);
  metricDef[5].mdMetricType=MD_CALCULATED|MD_INTERVAL;
  metricDef[5].mdDataType=MD_UINT64;
  metricDef[5].mdAliasId=metricDef[2].mdId;
  metricDef[5].mproc=NULL;
  metricDef[5].mdeal=NULL;
  metricDef[5].mcalc=metricCalcTotalCPUTime;
  metricDef[5].mresl=metricDef[0].mresl;
  metricDef[5].mresldeal=metricDef[0].mresldeal;

  metricDef[6].mdName="MemorySize";
  metricDef[6].mdId=mr(pluginname,metricDef[6].mdName);
  metricDef[6].mdSampleInterval=60;
  metricDef[6].mdMetricType=MD_RETRIEVED|MD_POINT;
  metricDef[6].mdDataType=MD_STRING;
  metricDef[6].mproc=metricRetrMemorySize;
  metricDef[6].mdeal=free;
  metricDef[6].mcalc=metricCalcMemorySize;
  metricDef[6].mresl=metricDef[0].mresl;
  metricDef[6].mresldeal=metricDef[0].mresldeal;

  metricDef[7].mdName="TotalVisibleMemorySize";
  metricDef[7].mdId=mr(pluginname,metricDef[7].mdName);
  metricDef[7].mdMetricType=MD_CALCULATED|MD_POINT;
  metricDef[7].mdDataType=MD_UINT64;
  metricDef[7].mdAliasId=metricDef[6].mdId;
  metricDef[7].mproc=NULL;
  metricDef[7].mdeal=NULL;
  metricDef[7].mcalc=metricCalcTotalPhysMem;
  metricDef[7].mresl=metricDef[0].mresl;
  metricDef[7].mresldeal=metricDef[0].mresldeal;

  metricDef[8].mdName="FreePhysicalMemory";
  metricDef[8].mdId=mr(pluginname,metricDef[8].mdName);
  metricDef[8].mdMetricType=MD_CALCULATED|MD_POINT;
  metricDef[8].mdDataType=MD_UINT64;
  metricDef[8].mdAliasId=metricDef[6].mdId;
  metricDef[8].mproc=NULL;
  metricDef[8].mdeal=NULL;
  metricDef[8].mcalc=metricCalcFreePhysMem;
  metricDef[8].mresl=metricDef[0].mresl;
  metricDef[8].mresldeal=metricDef[0].mresldeal;

  metricDef[9].mdName="SizeStoredInPagingFiles";
  metricDef[9].mdId=mr(pluginname,metricDef[9].mdName);
  metricDef[9].mdMetricType=MD_CALCULATED|MD_POINT;
  metricDef[9].mdDataType=MD_UINT64;
  metricDef[9].mdAliasId=metricDef[6].mdId;
  metricDef[9].mproc=NULL;
  metricDef[9].mdeal=NULL;
  metricDef[9].mcalc=metricCalcTotalSwapMem;
  metricDef[9].mresl=metricDef[0].mresl;
  metricDef[9].mresldeal=metricDef[0].mresldeal;

  metricDef[10].mdName="FreeSpaceInPagingFiles";
  metricDef[10].mdId=mr(pluginname,metricDef[10].mdName);
  metricDef[10].mdMetricType=MD_CALCULATED|MD_POINT;
  metricDef[10].mdDataType=MD_UINT64;
  metricDef[10].mdAliasId=metricDef[6].mdId;
  metricDef[10].mproc=NULL;
  metricDef[10].mdeal=NULL;
  metricDef[10].mcalc=metricCalcFreeSwapMem;
  metricDef[10].mresl=metricDef[0].mresl;
  metricDef[10].mresldeal=metricDef[0].mresldeal;

  metricDef[11].mdName="TotalVirtualMemorySize";
  metricDef[11].mdId=mr(pluginname,metricDef[11].mdName);
  metricDef[11].mdMetricType=MD_CALCULATED|MD_POINT;
  metricDef[11].mdDataType=MD_UINT64;
  metricDef[11].mdAliasId=metricDef[6].mdId;
  metricDef[11].mproc=NULL;
  metricDef[11].mdeal=NULL;
  metricDef[11].mcalc=metricCalcTotalVirtMem;
  metricDef[11].mresl=metricDef[0].mresl;
  metricDef[11].mresldeal=metricDef[0].mresldeal;

  metricDef[12].mdName="FreeVirtualMemory";
  metricDef[12].mdId=mr(pluginname,metricDef[12].mdName);
  metricDef[12].mdMetricType=MD_CALCULATED|MD_POINT;
  metricDef[12].mdDataType=MD_UINT64;
  metricDef[12].mdAliasId=metricDef[6].mdId;
  metricDef[12].mproc=NULL;
  metricDef[12].mdeal=NULL;
  metricDef[12].mcalc=metricCalcFreeVirtMem;
  metricDef[12].mresl=metricDef[0].mresl;
  metricDef[12].mresldeal=metricDef[0].mresldeal;

  metricDef[13].mdName="PageInCounter";
  metricDef[13].mdId=mr(pluginname,metricDef[13].mdName);
  metricDef[13].mdSampleInterval=60;
  metricDef[13].mdMetricType=MD_RETRIEVED|MD_POINT;
  metricDef[13].mdDataType=MD_UINT64;
  metricDef[13].mproc=metricRetrPageInCounter;
  metricDef[13].mdeal=free;
  metricDef[13].mcalc=metricCalcPageInCounter;
  metricDef[13].mresl=metricDef[0].mresl;
  metricDef[13].mresldeal=metricDef[0].mresldeal;

  metricDef[14].mdName="PageInRate";
  metricDef[14].mdId=mr(pluginname,metricDef[14].mdName);
  metricDef[14].mdMetricType=MD_CALCULATED|MD_RATE;
  metricDef[14].mdDataType=MD_UINT64;
  metricDef[14].mdAliasId=metricDef[13].mdId;
  metricDef[14].mproc=NULL;
  metricDef[14].mdeal=NULL;
  metricDef[14].mcalc=metricCalcPageInRate;
  metricDef[14].mresl=metricDef[0].mresl;
  metricDef[14].mresldeal=metricDef[0].mresldeal;

  metricDef[15].mdName="LoadCounter";
  metricDef[15].mdId=mr(pluginname,metricDef[15].mdName);
  metricDef[15].mdSampleInterval=60;
  metricDef[15].mdMetricType=MD_RETRIEVED|MD_POINT;
  metricDef[15].mdDataType=MD_FLOAT32;
  metricDef[15].mproc=metricRetrLoadCounter;
  metricDef[15].mdeal=free;
  metricDef[15].mcalc=metricCalcLoadCounter;
  metricDef[15].mresl=metricDef[0].mresl;
  metricDef[15].mresldeal=metricDef[0].mresldeal;

  metricDef[16].mdName="LoadAverage";
  metricDef[16].mdId=mr(pluginname,metricDef[16].mdName);
  metricDef[16].mdMetricType=MD_CALCULATED|MD_AVERAGE;
  metricDef[16].mdDataType=MD_FLOAT32;
  metricDef[16].mdAliasId=metricDef[15].mdId;
  metricDef[16].mproc=NULL;
  metricDef[16].mdeal=NULL;
  metricDef[16].mcalc=metricCalcLoadAverage;
  metricDef[16].mresl=metricDef[0].mresl;
  metricDef[16].mresldeal=metricDef[0].mresldeal;


  *mdnum=17;
  *md=metricDef;
  return 0;
}

int _StartStopMetrics (int starting) {

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : %s metric processing\n",
	  __FILE__,__LINE__,starting?"Starting":"Stopping");
#endif
  return 0;
}

int resourceLister(int mid, char *** list) {
  if (list) *list = resourceList;
  return 1;
}

void resourceListDeallocator(char ** list) { 
}


/* ---------------------------------------------------------------------------*/

int metricRetrNumOfUser( int mid, 
			 MetricReturner mret ) { 
  MetricValue * mv  = NULL;  
  char          str[255];
  int           fd_out[2]; 
  int           fd_stdout;
  int           fd_err[2]; 
  int           fd_stderr;
  int           rc  = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Retrieving NumberOfUsers\n",
	  __FILE__,__LINE__);
#endif
  if (mret==NULL) { fprintf(stderr,"Returner pointer is NULL\n"); }
  else {
#ifdef DEBUG
    fprintf(stderr,"--- %s(%i) : Sampling for metric NumberOfUsers ID %d\n",
	    __FILE__,__LINE__,mid);
#endif
    if( pipe(fd_out)==0 && pipe(fd_err)==0 ) {
      memset(str,0,sizeof(str));

      fd_stdout = dup( fileno(stdout) );
      close(fileno(stdout));
      dup2( fd_out[1], fileno(stdout) );

      fd_stderr = dup( fileno(stderr) );
      close(fileno(stderr));
      dup2( fd_err[1], fileno(stderr) );

      rc = system("who -u | wc -l");
      if( rc == 0 ) { read( fd_out[0], str, sizeof(str)-1 ); }
      else          { read( fd_err[0], str, sizeof(str)-1 ); }
	 
      close(fd_out[1]);
      dup2( fd_stdout, fileno(stdout) );
      close(fd_out[0]);
      close(fd_stdout);

      close(fd_err[1]);
      dup2( fd_stderr, fileno(stderr) );
      close(fd_err[0]);
      close(fd_stderr);
    }
    else { return -1; }
      
    mv = calloc(1, sizeof(MetricValue) + sizeof(unsigned long));
    if (mv) {
      mv->mvId = mid;
      mv->mvTimeStamp = time(NULL);
      mv->mvResource = resourceList[0];
      mv->mvDataType = MD_UINT32;
      mv->mvDataLength = sizeof(unsigned long);
      mv->mvData = (void*)mv + sizeof(MetricValue);
      *(unsigned long*)mv->mvData = atol(str);
      mret(mv);
    }
    
    return 1;
  }
  return -1;
}


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

int metricRetrNumOfProc( int mid, 
			 MetricReturner mret ) {
  MetricValue   * mv    = NULL;
  unsigned long   nop   = 0;
  DIR           * dir   = NULL;
  struct dirent * entry = NULL;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Retrieving NumberOfProcesses\n",
	  __FILE__,__LINE__);
#endif
  if (mret==NULL) { fprintf(stderr,"Returner pointer is NULL\n"); }
  else {
#ifdef DEBUG
    fprintf(stderr,"--- %s(%i) : Sampling for metric NumberOfProcesses ID %d\n",
	    __FILE__,__LINE__,mid);
#endif
    /* get number of processes */

    if( ( dir = opendir("/proc") ) != NULL ) {
      while( ( entry = readdir(dir)) != NULL ) {
	if( strcasecmp(entry->d_name,"1") == 0 ) { 
	  //	  fprintf(stderr,"entry : %s\n",entry->d_name);
	  nop++;
	  while( ( entry = readdir(dir)) != NULL ) {
	    nop++;
	    //	    fprintf(stderr,"entry : %s\n",entry->d_name);
	  }
	}
      }
      closedir(dir);
    }
    else { return -1; }

    mv = calloc(1, sizeof(MetricValue) + sizeof(unsigned long));
    if (mv) {
      mv->mvId = mid;
      mv->mvTimeStamp = time(NULL);
      mv->mvResource = resourceList[0];
      mv->mvDataType = MD_UINT32;
      mv->mvDataLength = sizeof(unsigned long);
      mv->mvData = (void*)mv + sizeof(MetricValue);
      *(unsigned long*)mv->mvData = nop;
      mret(mv);
    }
    return 1;
  }
  return -1;
}


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


/* ---------------------------------------------------------------------------*/

/* 
 * The raw data CPUTime has the following syntax :
 *
 * <user mode>:<user mode with low priority(nice)>:<system mode>:<idle task>
 *
 * the values in CPUTime are saved in Jiffies ( 1/100ths of a second )
 */

int metricRetrCPUTime( int mid, 
		       MetricReturner mret ) {
  MetricValue * mv  = NULL;
  FILE * fhd        = NULL;
  char   buf[30000];
  char * ptr        = NULL;
  char * end        = NULL;
  char * hlp        = NULL;
  size_t bytes_read = 0;
  int    i          = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Retrieving CPUTime\n",
	  __FILE__,__LINE__);
#endif
  if (mret==NULL) { fprintf(stderr,"Returner pointer is NULL\n"); }
  else {
#ifdef DEBUG
    fprintf(stderr,"--- %s(%i) : Sampling for metric CPUTime ID %d\n",
	    __FILE__,__LINE__,mid);
#endif
    if( (fhd = fopen("/proc/stat","r")) != NULL ) {

      bytes_read = fread(buf, 1, sizeof(buf)-1, fhd);
      ptr = strstr(buf,"cpu")+3;
      while( *ptr == ' ') { ptr++; }
      end = strchr(ptr, '\n');

      /* replace ' ' with ':' */
      hlp = ptr;
      for( ; i<3; i++ ) { 
	hlp = strchr(hlp, ' ');
	*hlp = ':';
      }
      fclose(fhd);
    }
    else { return -1; }

    mv = calloc( 1, sizeof(MetricValue) + 
		 (strlen(ptr)-strlen(end)+1) );
    if (mv) {
      mv->mvId = mid;
      mv->mvTimeStamp = time(NULL);
      mv->mvResource = resourceList[0];
      mv->mvDataType = MD_STRING;
      mv->mvDataLength = (strlen(ptr)-strlen(end)+1);
      mv->mvData = (void*)mv + sizeof(MetricValue);
      strncpy( mv->mvData, ptr, (strlen(ptr)-strlen(end)) );
      mret(mv);
    }
    return 1;
  }
  return -1;
}


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

    //fprintf(stderr,"kernel time: %lld\n",kt);
    memcpy(v,&kt,sizeof(unsigned long long));
    return sizeof(unsigned long long);
  }
  return -1;
}


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





/* ---------------------------------------------------------------------------*/

/* 
 * The raw data MemorySize has the following syntax :
 *
 * <TotalVisibleMemorySize>:<FreePhysicalMemory>:<SizeStoredInPagingFiles>:<FreeSpaceInPagingFiles>
 *
 * the values in MemorySize are saved in kBytes
 */


int metricRetrMemorySize( int mid, 
			  MetricReturner mret ) {
  MetricValue        * mv         = NULL;
  unsigned long long totalPhysMem = 0;
  unsigned long long freePhysMem  = 0;
  unsigned long long totalSwapMem = 0;
  unsigned long long freeSwapMem  = 0;
  FILE               * fhd        = NULL;
  char               * str        = NULL;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Retrieving MemorySize\n",
	  __FILE__,__LINE__);
#endif
  if (mret==NULL) { fprintf(stderr,"Returner pointer is NULL\n"); }
  else {
#ifdef DEBUG
    fprintf(stderr,"--- %s(%i) : Sampling for metric MemorySize ID %d\n",
	    __FILE__,__LINE__,mid);
#endif
    if ( (fhd=fopen("/proc/meminfo","r")) != NULL ) {
      fscanf(fhd,
	     "%*s %*s %*s %*s %*s %*s %*s %lld %*s %lld %*s %*s %*s %*s %lld %*s %lld",
	     &totalPhysMem,&freePhysMem,
	     &totalSwapMem,&freeSwapMem );
      fclose(fhd);
    }
    else { return -1; }

    totalPhysMem = totalPhysMem/1024;
    freePhysMem  = freePhysMem/1024;
    totalSwapMem = totalSwapMem/1024;
    freeSwapMem  = freeSwapMem/1024;
    
    str = calloc(1, ((4*sizeof(unsigned long long))+4) );
    sprintf( str,"%lld:%lld:%lld:%lld",
	     totalPhysMem,freePhysMem,totalSwapMem,freeSwapMem);
    
    mv = calloc( 1, sizeof(MetricValue) + 
		 (strlen(str)+1) );
    if (mv) {
      mv->mvId = mid;
      mv->mvTimeStamp = time(NULL);
      mv->mvResource = resourceList[0];
      mv->mvDataType = MD_STRING;
      mv->mvDataLength = strlen(str)+1;
      mv->mvData = (void*)mv + sizeof(MetricValue);
      strcpy( mv->mvData, str );
      mret(mv);
    }

    if(str) free(str);
    return 1;
  }
  return -1;
}


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

int metricRetrPageInCounter( int mid, 
			     MetricReturner mret ) { 
  MetricValue      * mv   = NULL; 
  FILE             * fhd  = NULL;
  unsigned long long page = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Retrieving PageInCounter\n",
	  __FILE__,__LINE__);
#endif
  if (mret==NULL) { fprintf(stderr,"Returner pointer is NULL\n"); }
  else {
#ifdef DEBUG
    fprintf(stderr,"--- %s(%i) : Sampling for metric PageInCounter ID %d\n",
	    __FILE__,__LINE__,mid);
#endif
    if ( (fhd=fopen("/proc/stat","r")) != NULL ) {
      fscanf(fhd,
	     "%*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %lld",
	     &page);
      fclose(fhd);
    }
    else { return -1; }

    mv = calloc(1, sizeof(MetricValue) + sizeof(unsigned long long));
    if (mv) {
      mv->mvId = mid;
      mv->mvTimeStamp = time(NULL);
      mv->mvResource = resourceList[0];
      mv->mvDataType = MD_UINT64;
      mv->mvDataLength = sizeof(unsigned long long);
      mv->mvData = (void*)mv + sizeof(MetricValue);
      *(unsigned long long*)mv->mvData = page;
      mret(mv);
    }
      
    return 1;
  }
  return -1;
}


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
/* LoadCounter, LoadAverage                                                   */
/* ---------------------------------------------------------------------------*/

int metricRetrLoadCounter( int mid, 
			   MetricReturner mret ) { 
  MetricValue * mv   = NULL; 
  FILE        * fhd  = NULL;
  float         load = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Retrieving LoadCounter\n",
	  __FILE__,__LINE__);
#endif
  if (mret==NULL) { fprintf(stderr,"Returner pointer is NULL\n"); }
  else {
#ifdef DEBUG
    fprintf(stderr,"--- %s(%i) : Sampling for metric LoadCounter ID %d\n",
	    __FILE__,__LINE__,mid);
#endif
    if ( (fhd=fopen("/proc/loadavg","r")) != NULL ) {
      fscanf(fhd, 
	     "%f",
	     &load);
      fclose(fhd);
    }
    else { return -1; }

    mv = calloc(1, sizeof(MetricValue) + sizeof(float));
    if (mv) {
      mv->mvId = mid;
      mv->mvTimeStamp = time(NULL);
      mv->mvResource = resourceList[0];
      mv->mvDataType = MD_FLOAT32;
      mv->mvDataLength = sizeof(float);
      mv->mvData = (void*)mv + sizeof(MetricValue);
      *(float*)mv->mvData = load;
      mret(mv);
    }
    return 1;
  }
  return -1;
}


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
