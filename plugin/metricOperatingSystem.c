/*
 * $Id: metricOperatingSystem.c,v 1.7 2004/08/04 09:00:04 heidineu Exp $
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
 * Metrics Gatherer Plugin of the following Operating System specific metrics :
 *
 * NumberOfUsers
 * NumberOfProcesses
 * CPUTime
 * MemorySize
 * PageInCounter
 * LoadCounter
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

static MetricDefinition  metricDef[6];
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

int _DefinedMetrics( MetricRegisterId *mr,
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
  metricDef[0].mdReposPluginName="librepositoryOperatingSystem.so";
  metricDef[0].mdId=mr(pluginname,metricDef[0].mdName);
  metricDef[0].mdSampleInterval=60;
  metricDef[0].mproc=metricRetrNumOfUser;
  metricDef[0].mdeal=free;

  metricDef[1].mdVersion=MD_VERSION;
  metricDef[1].mdName="NumberOfProcesses";
  metricDef[1].mdReposPluginName="librepositoryOperatingSystem.so";
  metricDef[1].mdId=mr(pluginname,metricDef[1].mdName);
  metricDef[1].mdSampleInterval=60;
  metricDef[1].mproc=metricRetrNumOfProc;
  metricDef[1].mdeal=free;

  metricDef[2].mdVersion=MD_VERSION;
  metricDef[2].mdName="CPUTime";
  metricDef[2].mdReposPluginName="librepositoryOperatingSystem.so";
  metricDef[2].mdId=mr(pluginname,metricDef[2].mdName);
  metricDef[2].mdSampleInterval=60;
  metricDef[2].mproc=metricRetrCPUTime;
  metricDef[2].mdeal=free;

  metricDef[3].mdVersion=MD_VERSION;
  metricDef[3].mdName="MemorySize";
  metricDef[3].mdReposPluginName="librepositoryOperatingSystem.so";
  metricDef[3].mdId=mr(pluginname,metricDef[3].mdName);
  metricDef[3].mdSampleInterval=60;
  metricDef[3].mproc=metricRetrMemorySize;
  metricDef[3].mdeal=free;

  metricDef[4].mdVersion=MD_VERSION;
  metricDef[4].mdName="PageInCounter";
  metricDef[4].mdReposPluginName="librepositoryOperatingSystem.so";
  metricDef[4].mdId=mr(pluginname,metricDef[4].mdName);
  metricDef[4].mdSampleInterval=60;
  metricDef[4].mproc=metricRetrPageInCounter;
  metricDef[4].mdeal=free;

  metricDef[5].mdVersion=MD_VERSION;
  metricDef[5].mdName="LoadCounter";
  metricDef[5].mdReposPluginName="librepositoryOperatingSystem.so";
  metricDef[5].mdId=mr(pluginname,metricDef[5].mdName);
  metricDef[5].mdSampleInterval=60;
  metricDef[5].mproc=metricRetrLoadCounter;
  metricDef[5].mdeal=free;

  *mdnum=6;
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
      
    mv = calloc(1, sizeof(MetricValue) + 
		   sizeof(unsigned long) + 
		   (strlen(resource)+1) );
    if (mv) {
      mv->mvId = mid;
      mv->mvTimeStamp = time(NULL);
      mv->mvDataType = MD_UINT32;
      mv->mvDataLength = sizeof(unsigned long);
      mv->mvData = (void*)mv + sizeof(MetricValue);
      *(unsigned long*)mv->mvData = atol(str);
      mv->mvResource = (void*)mv + sizeof(MetricValue) + sizeof(unsigned long);
      strcpy(mv->mvResource,resource);
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

    mv = calloc(1, sizeof(MetricValue) + 
		   sizeof(unsigned long) + 
		   (strlen(resource)+1) );
    if (mv) {
      mv->mvId = mid;
      mv->mvTimeStamp = time(NULL);
      mv->mvDataType = MD_UINT32;
      mv->mvDataLength = sizeof(unsigned long);
      mv->mvData = (void*)mv + sizeof(MetricValue);
      *(unsigned long*)mv->mvData = nop;
      mv->mvResource = (void*)mv + sizeof(MetricValue) + sizeof(unsigned long);
      strcpy(mv->mvResource,resource);
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
		    (strlen(ptr)-strlen(end)+1)  + 
		    (strlen(resource)+1) );
    if (mv) {
      mv->mvId = mid;
      mv->mvTimeStamp = time(NULL);
      mv->mvDataType = MD_STRING;
      mv->mvDataLength = (strlen(ptr)-strlen(end)+1);
      mv->mvData = (void*)mv + sizeof(MetricValue);
      strncpy( mv->mvData, ptr, (strlen(ptr)-strlen(end)) );
      mv->mvResource = (void*)mv + sizeof(MetricValue) + ((strlen(ptr)-strlen(end))+1);
      strcpy(mv->mvResource,resource);
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
		    (strlen(str)+1)  + 
		    (strlen(resource)+1) );
    if (mv) {
      mv->mvId = mid;
      mv->mvTimeStamp = time(NULL);
      mv->mvDataType = MD_STRING;
      mv->mvDataLength = strlen(str)+1;
      mv->mvData = (void*)mv + sizeof(MetricValue);
      strcpy( mv->mvData, str );
      mv->mvResource = (void*)mv + sizeof(MetricValue) + strlen(str) +1;
      strcpy(mv->mvResource,resource);
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

    mv = calloc(1, sizeof(MetricValue) + 
		   sizeof(unsigned long long) + 
		   (strlen(resource)+1) );
    if (mv) {
      mv->mvId = mid;
      mv->mvTimeStamp = time(NULL);
      mv->mvDataType = MD_UINT64;
      mv->mvDataLength = sizeof(unsigned long long);
      mv->mvData = (void*)mv + sizeof(MetricValue);
      *(unsigned long long*)mv->mvData = page;
      mv->mvResource = (void*)mv + sizeof(MetricValue) + sizeof(unsigned long long);
      strcpy(mv->mvResource,resource);
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

    mv = calloc(1, sizeof(MetricValue) +
		   sizeof(float) + 
		   (strlen(resource)+1) );
    if (mv) {
      mv->mvId = mid;
      mv->mvTimeStamp = time(NULL);
      mv->mvDataType = MD_FLOAT32;
      mv->mvDataLength = sizeof(float);
      mv->mvData = (void*)mv + sizeof(MetricValue);
      *(float*)mv->mvData = load;
      mv->mvResource = (void*)mv + sizeof(MetricValue) + sizeof(float);
      strcpy(mv->mvResource,resource);
      mret(mv);
    }
    return 1;
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/
/*                    end of metricOperatingSystem.c                          */
/* ---------------------------------------------------------------------------*/
