/*
 * $Id: metricOperatingSystem.c,v 1.5 2004/08/03 10:23:54 heidineu Exp $
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
static MetricDeallocator valueDeallocator;

/* --- NumberOfUsers --- */
static MetricRetriever   metricRetrNumOfUser;

/* --- NumberOfProcesses --- */
static MetricRetriever   metricRetrNumOfProc;

/* --- CPUTime is base for :
 * KernelModeTime, UserModeTime, TotalCPUTime --- */
static MetricRetriever   metricRetrCPUTime;

/* --- MemorySize is base for :
 * TotalVisibleMemorySize,  FreePhysicalMemory, 
 * SizeStoredInPagingFiles, FreeSpaceInPagingFiles,
 * TotalVirtualMemorySize,  FreeVirtualMemory --- */
static MetricRetriever   metricRetrMemorySize;

/* --- PageInCounter, PageInRate --- */
static MetricRetriever   metricRetrPageInCounter;

/* --- LoadCounter, LoadAverage --- */
static MetricRetriever   metricRetrLoadCounter;

/* ---------------------------------------------------------------------------*/

static char * resource = "OperatingSystem";

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

  metricDef[0].mdVersion=MD_VERSION;
  metricDef[0].mdName="NumberOfUsers";
  metricDef[0].mdReposPluginName="plugin/librepositoryOperatingSystem.so";
  metricDef[0].mdId=mr(pluginname,metricDef[0].mdName);
  metricDef[0].mdSampleInterval=60;
  metricDef[0].mdMetricType=MD_RETRIEVED|MD_POINT;
  metricDef[0].mdDataType=MD_UINT32;
  metricDef[0].mproc=metricRetrNumOfUser;
  metricDef[0].mdeal = valueDeallocator;

  metricDef[1].mdVersion=MD_VERSION;
  metricDef[1].mdName="NumberOfProcesses";
  metricDef[1].mdReposPluginName="plugin/librepositoryOperatingSystem.so";
  metricDef[1].mdId=mr(pluginname,metricDef[1].mdName);
  metricDef[1].mdSampleInterval=60;
  metricDef[1].mdMetricType=MD_RETRIEVED|MD_POINT;
  metricDef[1].mdDataType=MD_UINT32;
  metricDef[1].mproc=metricRetrNumOfProc;
  metricDef[1].mdeal = valueDeallocator;

  metricDef[2].mdVersion=MD_VERSION;
  metricDef[2].mdName="CPUTime";
  metricDef[2].mdReposPluginName="plugin/librepositoryOperatingSystem.so";
  metricDef[2].mdId=mr(pluginname,metricDef[2].mdName);
  metricDef[2].mdSampleInterval=60;
  metricDef[2].mdMetricType=MD_RETRIEVED|MD_POINT;
  metricDef[2].mdDataType=MD_STRING;
  metricDef[2].mproc=metricRetrCPUTime;
  metricDef[2].mdeal = valueDeallocator;

  metricDef[3].mdVersion=MD_VERSION;
  metricDef[3].mdName="KernelModeTime";
  metricDef[3].mdReposPluginName="plugin/librepositoryOperatingSystem.so";
  metricDef[3].mdId=mr(pluginname,metricDef[3].mdName);
  metricDef[3].mdMetricType=MD_CALCULATED|MD_INTERVAL;
  metricDef[3].mdDataType=MD_UINT64;
  metricDef[3].mproc=NULL;
  metricDef[3].mdeal=NULL;

  metricDef[4].mdVersion=MD_VERSION;
  metricDef[4].mdName="UserModeTime";
  metricDef[4].mdReposPluginName="plugin/librepositoryOperatingSystem.so";
  metricDef[4].mdId=mr(pluginname,metricDef[4].mdName);
  metricDef[4].mdMetricType=MD_CALCULATED|MD_INTERVAL;
  metricDef[4].mdDataType=MD_UINT64;
  metricDef[4].mproc=NULL;
  metricDef[4].mdeal=NULL;

  metricDef[5].mdVersion=MD_VERSION;
  metricDef[5].mdName="TotalCPUTime";
  metricDef[5].mdReposPluginName="plugin/librepositoryOperatingSystem.so";
  metricDef[5].mdId=mr(pluginname,metricDef[5].mdName);
  metricDef[5].mdMetricType=MD_CALCULATED|MD_INTERVAL;
  metricDef[5].mdDataType=MD_UINT64;
  metricDef[5].mproc=NULL;
  metricDef[5].mdeal=NULL;

  metricDef[6].mdVersion=MD_VERSION;
  metricDef[6].mdName="MemorySize";
  metricDef[6].mdReposPluginName="plugin/librepositoryOperatingSystem.so";
  metricDef[6].mdId=mr(pluginname,metricDef[6].mdName);
  metricDef[6].mdSampleInterval=60;
  metricDef[6].mdMetricType=MD_RETRIEVED|MD_POINT;
  metricDef[6].mdDataType=MD_STRING;
  metricDef[6].mproc=metricRetrMemorySize;
  metricDef[6].mdeal = valueDeallocator;

  metricDef[7].mdVersion=MD_VERSION;
  metricDef[7].mdName="TotalVisibleMemorySize";
  metricDef[7].mdReposPluginName="plugin/librepositoryOperatingSystem.so";
  metricDef[7].mdId=mr(pluginname,metricDef[7].mdName);
  metricDef[7].mdMetricType=MD_CALCULATED|MD_POINT;
  metricDef[7].mdDataType=MD_UINT64;
  metricDef[7].mproc=NULL;
  metricDef[7].mdeal=NULL;

  metricDef[8].mdVersion=MD_VERSION;
  metricDef[8].mdName="FreePhysicalMemory";
  metricDef[8].mdReposPluginName="plugin/librepositoryOperatingSystem.so";
  metricDef[8].mdId=mr(pluginname,metricDef[8].mdName);
  metricDef[8].mdMetricType=MD_CALCULATED|MD_POINT;
  metricDef[8].mdDataType=MD_UINT64;
  metricDef[8].mproc=NULL;
  metricDef[8].mdeal=NULL;

  metricDef[9].mdVersion=MD_VERSION;
  metricDef[9].mdName="SizeStoredInPagingFiles";
  metricDef[9].mdReposPluginName="plugin/librepositoryOperatingSystem.so";
  metricDef[9].mdId=mr(pluginname,metricDef[9].mdName);
  metricDef[9].mdMetricType=MD_CALCULATED|MD_POINT;
  metricDef[9].mdDataType=MD_UINT64;
  metricDef[9].mproc=NULL;
  metricDef[9].mdeal=NULL;

  metricDef[10].mdVersion=MD_VERSION;
  metricDef[10].mdName="FreeSpaceInPagingFiles";
  metricDef[10].mdReposPluginName="plugin/librepositoryOperatingSystem.so";
  metricDef[10].mdId=mr(pluginname,metricDef[10].mdName);
  metricDef[10].mdMetricType=MD_CALCULATED|MD_POINT;
  metricDef[10].mdDataType=MD_UINT64;
  metricDef[10].mproc=NULL;
  metricDef[10].mdeal=NULL;

  metricDef[11].mdVersion=MD_VERSION;
  metricDef[11].mdName="TotalVirtualMemorySize";
  metricDef[11].mdReposPluginName="plugin/librepositoryOperatingSystem.so";
  metricDef[11].mdId=mr(pluginname,metricDef[11].mdName);
  metricDef[11].mdMetricType=MD_CALCULATED|MD_POINT;
  metricDef[11].mdDataType=MD_UINT64;
  metricDef[11].mproc=NULL;
  metricDef[11].mdeal=NULL;

  metricDef[12].mdVersion=MD_VERSION;
  metricDef[12].mdName="FreeVirtualMemory";
  metricDef[12].mdReposPluginName="plugin/librepositoryOperatingSystem.so";
  metricDef[12].mdId=mr(pluginname,metricDef[12].mdName);
  metricDef[12].mdMetricType=MD_CALCULATED|MD_POINT;
  metricDef[12].mdDataType=MD_UINT64;
  metricDef[12].mproc=NULL;
  metricDef[12].mdeal=NULL;

  metricDef[13].mdVersion=MD_VERSION;
  metricDef[13].mdName="PageInCounter";
  metricDef[13].mdReposPluginName="plugin/librepositoryOperatingSystem.so";
  metricDef[13].mdId=mr(pluginname,metricDef[13].mdName);
  metricDef[13].mdSampleInterval=60;
  metricDef[13].mdMetricType=MD_RETRIEVED|MD_POINT;
  metricDef[13].mdDataType=MD_UINT64;
  metricDef[13].mproc=metricRetrPageInCounter;
  metricDef[13].mdeal = valueDeallocator;

  metricDef[14].mdVersion=MD_VERSION;
  metricDef[14].mdName="PageInRate";
  metricDef[14].mdReposPluginName="plugin/librepositoryOperatingSystem.so";
  metricDef[14].mdId=mr(pluginname,metricDef[14].mdName);
  metricDef[14].mdMetricType=MD_CALCULATED|MD_RATE;
  metricDef[14].mdDataType=MD_UINT64;
  metricDef[14].mproc=NULL;
  metricDef[14].mdeal=NULL;

  metricDef[15].mdVersion=MD_VERSION;
  metricDef[15].mdName="LoadCounter";
  metricDef[15].mdReposPluginName="plugin/librepositoryOperatingSystem.so";
  metricDef[15].mdId=mr(pluginname,metricDef[15].mdName);
  metricDef[15].mdSampleInterval=60;
  metricDef[15].mdMetricType=MD_RETRIEVED|MD_POINT;
  metricDef[15].mdDataType=MD_FLOAT32;
  metricDef[15].mproc=metricRetrLoadCounter;
  metricDef[15].mdeal = valueDeallocator;

  metricDef[16].mdVersion=MD_VERSION;
  metricDef[16].mdName="LoadAverage";
  metricDef[16].mdReposPluginName="plugin/librepositoryOperatingSystem.so";
  metricDef[16].mdId=mr(pluginname,metricDef[16].mdName);
  metricDef[16].mdMetricType=MD_CALCULATED|MD_AVERAGE;
  metricDef[16].mdDataType=MD_FLOAT32;
  metricDef[16].mproc=NULL;
  metricDef[16].mdeal=NULL;

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


void valueDeallocator(void *v)
{
    MetricValue* mv = (MetricValue *)v;
    if (mv) {
	if (mv->mvResource) free(mv->mvResource);
	free(mv);
    }
}


/* ---------------------------------------------------------------------------*/
/* NumberOfUsers                                                              */
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
      mv->mvResource = strdup(resource);
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


/* ---------------------------------------------------------------------------*/
/* NumberOfProcesses                                                          */
/* ---------------------------------------------------------------------------*/

int metricRetrNumOfProc( int mid, 
			 MetricReturner mret ) {
  MetricValue   * mv    = NULL;
  char          * value = NULL;
  char          * ptr   = NULL;
  unsigned long   nop   = 0;
  FILE          * fhd   = NULL;

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
    if ( (fhd=fopen("/proc/loadavg","r")) != NULL ) {
      value = calloc(1,256);
      fscanf(fhd, 
	     "%*s %*s %*s %s",
	     value);
      fclose(fhd);
      ptr = strchr(value,'/');
      ptr++;
      nop = atol(ptr);
      if(value) { free(value); }
    }
    else { return -1; }

    mv = calloc(1, sizeof(MetricValue) + sizeof(unsigned long));
    if (mv) {
      mv->mvId = mid;
      mv->mvTimeStamp = time(NULL);
      mv->mvResource = strdup(resource);
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
      mv->mvResource = strdup(resource);
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
      mv->mvResource = strdup(resource);
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


/* ---------------------------------------------------------------------------*/
/* PageInCounter                                                              */
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
	     "%*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %lld",
	     &page);
      fclose(fhd);
    }
    else { return -1; }

    mv = calloc(1, sizeof(MetricValue) + sizeof(unsigned long long));
    if (mv) {
      mv->mvId = mid;
      mv->mvTimeStamp = time(NULL);
      mv->mvResource = strdup(resource);
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


/* ---------------------------------------------------------------------------*/
/* LoadCounter                                                                */
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
      mv->mvResource = strdup(resource);
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


/* ---------------------------------------------------------------------------*/
/*                    end of metricOperatingSystem.c                          */
/* ---------------------------------------------------------------------------*/
