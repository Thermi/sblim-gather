/*
 * $Id: metricUnixProcess.c,v 1.5 2004/08/19 10:54:59 heidineu Exp $
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
 * Metrics Gatherer Plugin of the following Unix Process specific metrics :
 *
 * KernelModeTime
 * UserModeTime
 * TotalCPUTime
 * ResidentSetSize
 * PageInCounter
 * PageInRate
 * InternalViewKernelModePercentage
 * InternalViewUserModePercentage
 * InternalViewTotalCPUPercentage
 * ExternalViewKernelModePercentage
 * ExternalViewUserModePercentage
 * ExternalViewTotalCPUPercentage
 * AccumulatedKernelModeTime
 * AccumulatedUserModeTime
 * AccumulatedTotalCPUTime
 * VirtualSize
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

static MetricDefinition  metricDef[4];

/* --- CPUTime is base for :
 * KernelModeTime, UserModeTime, TotalCPUTime,
 * InternalViewKernelModePercentage, InternalViewUserModePercentage,
 * InternalViewTotalCPUPercentage, ExternalViewKernelModePercentage,
 * ExternalViewUserModePercentage, ExternalViewTotalCPUPercentage,
 * AccumulatedKernelModeTime, AccumulatedUserModeTime,
 * AccumulatedTotalCPUTime
 * --- */
static MetricRetriever   metricRetrCPUTime;

/* --- ResidentSetSize --- */
static MetricRetriever   metricRetrResSetSize;

/* --- PageInCounter, PageInRate --- */
static MetricRetriever   metricRetrPageInCounter;

/* --- VirtualSize --- */
static MetricRetriever   metricRetrVirtualSize;

/* ---------------------------------------------------------------------------*/

static int enum_all_pid( char ** list );

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
  metricDef[0].mdName="CPUTime";
  metricDef[0].mdReposPluginName="librepositoryUnixProcess.so";
  metricDef[0].mdId=mr(pluginname,metricDef[0].mdName);
  metricDef[0].mdSampleInterval=60;
  metricDef[0].mproc=metricRetrCPUTime;
  metricDef[0].mdeal=free;

  metricDef[1].mdVersion=MD_VERSION;
  metricDef[1].mdName="ResidentSetSize";
  metricDef[1].mdReposPluginName="librepositoryUnixProcess.so";
  metricDef[1].mdId=mr(pluginname,metricDef[1].mdName);
  metricDef[1].mdSampleInterval=60;
  metricDef[1].mproc=metricRetrResSetSize;
  metricDef[1].mdeal=free;

  metricDef[2].mdVersion=MD_VERSION;
  metricDef[2].mdName="PageInCounter";
  metricDef[2].mdReposPluginName="librepositoryUnixProcess.so";
  metricDef[2].mdId=mr(pluginname,metricDef[2].mdName);
  metricDef[2].mdSampleInterval=60;
  metricDef[2].mproc=metricRetrPageInCounter;
  metricDef[2].mdeal=free;

  metricDef[3].mdVersion=MD_VERSION;
  metricDef[3].mdName="VirtualSize";
  metricDef[3].mdReposPluginName="librepositoryUnixProcess.so";
  metricDef[3].mdId=mr(pluginname,metricDef[3].mdName);
  metricDef[3].mdSampleInterval=60;
  metricDef[3].mproc=metricRetrVirtualSize;
  metricDef[3].mdeal=free;

  *mdnum=4;
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
/* CPUTime                                                                    */
/* ---------------------------------------------------------------------------*/

/* 
 * The raw data CPUTime has the following syntax :
 *
 * <PID user mode time>:<PID kernel mode time>:<OS user mode>:
 * <OS user mode with low priority(nice)>:<OS system mode>:<OS idle task>
 *
 * the values in CPUTime are saved in Jiffies ( 1/100ths of a second )
 */

int metricRetrCPUTime( int mid, 
		       MetricReturner mret ) {
  MetricValue * mv          = NULL;
  FILE        * fhd         = NULL;
  char        * _enum_pid   = NULL;
  char        * ptr         = NULL;
  char        * end         = NULL;
  char        * hlp         = NULL;
  char          buf[4096];
  char          os_buf[4096];
  int           _enum_size  = 0;
  int           i           = 0;
  size_t        bytes_read  = 0;
  unsigned long long u_time = 0;
  unsigned long long k_time = 0;

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

    /* get OS specific CPUTime */
    if( (fhd = fopen("/proc/stat","r")) != NULL ) {
      bytes_read = fread(os_buf, 1, sizeof(os_buf)-1, fhd);
      ptr = strstr(os_buf,"cpu")+3;
      while( *ptr == ' ') { ptr++; }
      end = strchr(ptr, '\n');
      hlp = ptr;
      for( ; i<3; i++ ) { 
	hlp = strchr(hlp, ' ');
	*hlp = ':';
      }
      fclose(fhd);
    }
    else { return -1; }
    fhd=NULL;

    /* get number of processes */
    _enum_size = enum_all_pid( &_enum_pid );
    if( _enum_size > 0 ) {
      for(i=0;i<_enum_size;i++) {

	u_time = 0;
	k_time = 0;
	    
	memset(buf,0,sizeof(buf));
	strcpy(buf,"/proc/");
	strcat(buf,_enum_pid + (i*64));
	strcat(buf,"/stat");
	if( (fhd = fopen(buf,"r")) != NULL ) {
	  fscanf(fhd,"%*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %lld %lld", 
		 &u_time,&k_time );
	  fclose(fhd);
	}

	memset(buf,0,sizeof(buf));
	sprintf(buf,"%lld:%lld:",u_time,k_time);
	strncpy(buf+strlen(buf),ptr,strlen(ptr)-strlen(end));

	mv = calloc( 1, sizeof(MetricValue) + 
		     (strlen(buf)+1) +
		     (strlen(_enum_pid+(i*64))+1) );
	if (mv) {
	  mv->mvId = mid;
	  mv->mvTimeStamp = time(NULL);
	  mv->mvDataType = MD_STRING;
	  mv->mvDataLength = (strlen(buf)+1);
	  mv->mvData = (void*)mv + sizeof(MetricValue);
	  strncpy( mv->mvData, buf, strlen(buf) );	
	  mv->mvResource = (void*)mv + sizeof(MetricValue) + (strlen(buf)+1);
	  strcpy(mv->mvResource,_enum_pid+(i*64));
	  mret(mv);
	}
      }
      if(_enum_pid) free(_enum_pid);
      return _enum_size;
    }
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/
/* ResidentSetSize                                                            */
/* ---------------------------------------------------------------------------*/

int metricRetrResSetSize( int mid, 
			  MetricReturner mret ) {
  MetricValue   * mv         = NULL;
  FILE          * fhd        = NULL;
  char          * _enum_pid  = NULL;
  char            buf[254];
  int             _enum_size = 0;
  int             i          = 0;
  unsigned long long size    = 0;
  unsigned long long rss     = 0;
  unsigned long long rlim    = 0;


#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Retrieving ResidentSetSize\n",
	  __FILE__,__LINE__);
#endif
  if (mret==NULL) { fprintf(stderr,"Returner pointer is NULL\n"); }
  else {
#ifdef DEBUG
    fprintf(stderr,"--- %s(%i) : Sampling for metric ResidentSetSize ID %d\n",
	    __FILE__,__LINE__,mid);
#endif
    /* get number of processes */
    _enum_size = enum_all_pid( &_enum_pid );
    if( _enum_size > 0 ) {
      for(i=0;i<_enum_size;i++) {

	size = 0;
	memset(buf,0,sizeof(buf));
	strcpy(buf,"/proc/");
	strcat(buf,_enum_pid + (i*64));
	strcat(buf,"/stat");
	if( (fhd = fopen(buf,"r")) != NULL ) {
	  fscanf(fhd,"%*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s "
		 "%*s %*s %*s %*s %*s %*s %*s %*s %*s %lld %lld",
		 &rss, &rlim );
	  fclose(fhd);
	  size = rss * rlim;
	}
	
	mv = calloc( 1, sizeof(MetricValue) + 
		     sizeof(unsigned long long) +
		     (strlen(_enum_pid + (i*64))+1) );
	if (mv) {
	  mv->mvId = mid;
	  mv->mvTimeStamp = time(NULL);
	  mv->mvDataType = MD_UINT64;
	  mv->mvDataLength = sizeof(unsigned long long);
	  mv->mvData = (void*)mv + sizeof(MetricValue);
	  *(unsigned long long *)mv->mvData = size;	
	  mv->mvResource = (void*)mv + sizeof(MetricValue) + sizeof(unsigned long long);
	  strcpy(mv->mvResource,_enum_pid + (i*64));
	  mret(mv);
	}
      }
      if(_enum_pid) free(_enum_pid);
      return _enum_size;
    }
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/
/* PageInCounter                                                              */
/* ---------------------------------------------------------------------------*/

int metricRetrPageInCounter( int mid, 
			     MetricReturner mret ) { 
  MetricValue      * mv         = NULL; 
  FILE             * fhd        = NULL;
  char             * _enum_pid  = NULL;
  char               buf[254];
  int                _enum_size = 0;
  int                i          = 0;
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

    /* get number of processes */
    _enum_size = enum_all_pid( &_enum_pid );
    if( _enum_size > 0 ) {
      for(i=0;i<_enum_size;i++) {

	page = 0;
	memset(buf,0,sizeof(buf));
	strcpy(buf,"/proc/");
	strcat(buf,_enum_pid + (i*64));
	strcat(buf,"/stat");
	if( (fhd = fopen(buf,"r")) != NULL ) {
	  fscanf(fhd,
	     "%*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %lld",
	     &page);
	  fclose(fhd);
	}

	mv = calloc( 1, sizeof(MetricValue) + 
		     sizeof(unsigned long long) +
		     (strlen(_enum_pid + (i*64))+1) );
	if (mv) {
	  mv->mvId = mid;
	  mv->mvTimeStamp = time(NULL);
	  mv->mvDataType = MD_UINT64;
	  mv->mvDataLength = sizeof(unsigned long long);
	  mv->mvData = (void*)mv + sizeof(MetricValue);
	  *(unsigned long long*)mv->mvData = page;	 
	  mv->mvResource = (void*)mv + sizeof(MetricValue) + sizeof(unsigned long long);
	  strcpy(mv->mvResource,_enum_pid + (i*64));
	  mret(mv);
	}	
      }
      if(_enum_pid) free(_enum_pid);
      return _enum_size;
    }
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/
/* VirtualSize                                                                */
/* ---------------------------------------------------------------------------*/

int metricRetrVirtualSize( int mid, 
			   MetricReturner mret ) {
  MetricValue   * mv         = NULL;
  FILE          * fhd        = NULL;
  char          * _enum_pid  = NULL;
  char            buf[254];
  int             _enum_size = 0;
  int             i          = 0;
  unsigned long long size    = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Retrieving VirtualSize\n",
	  __FILE__,__LINE__);
#endif
  if (mret==NULL) { fprintf(stderr,"Returner pointer is NULL\n"); }
  else {
#ifdef DEBUG
    fprintf(stderr,"--- %s(%i) : Sampling for metric VirtualSize ID %d\n",
	    __FILE__,__LINE__,mid);
#endif
    /* get number of processes */
    _enum_size = enum_all_pid( &_enum_pid );
    if( _enum_size > 0 ) {
      for(i=0;i<_enum_size;i++) {

	size = 0;
	memset(buf,0,sizeof(buf));
	strcpy(buf,"/proc/");
	strcat(buf,_enum_pid + (i*64));
	strcat(buf,"/stat");
	if( (fhd = fopen(buf,"r")) != NULL ) {
	  fscanf(fhd,"%*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s "
		 "%*s %*s %*s %*s %*s %*s %*s %*s %lld",
		 &size);
	  fclose(fhd);
	}
	
	mv = calloc( 1, sizeof(MetricValue) + 
		     sizeof(unsigned long long) +
		     (strlen(_enum_pid + (i*64))+1) );
	if (mv) {
	  mv->mvId = mid;
	  mv->mvTimeStamp = time(NULL);
	  mv->mvDataType = MD_UINT64;
	  mv->mvDataLength = sizeof(unsigned long long);
	  mv->mvData = (void*)mv + sizeof(MetricValue);
	  *(unsigned long long *)mv->mvData = size;	
	  mv->mvResource = (void*)mv + sizeof(MetricValue) + sizeof(unsigned long long);
	  strcpy(mv->mvResource,_enum_pid + (i*64));
	  mret(mv);
	}
      }
      if(_enum_pid) free(_enum_pid);
      return _enum_size;
    }
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/
/* get all Process IDs                                                        */
/* ---------------------------------------------------------------------------*/

int enum_all_pid( char ** list ) {  

  struct dirent * entry = NULL;
  DIR           * dir   = NULL;
  char          * _enum_pid = NULL;
  int             _enum_size = 0;
  int             i     = 1;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : enum_all_pid()\n",__FILE__,__LINE__);
#endif
  /* get number of processes */
  if( ( dir = opendir("/proc") ) != NULL ) {
    while( ( entry = readdir(dir)) != NULL ) {

      if( strcasecmp(entry->d_name,"1") == 0 ) { 
	_enum_size = 1;
	_enum_pid  = calloc(_enum_size,64);
	strcpy(_enum_pid,entry->d_name);
	while( ( entry = readdir(dir)) != NULL ) {
	  if( strncmp(entry->d_name,".",1) != 0 ) {
	    if( i==_enum_size ) {
	      _enum_size++;
	      _enum_pid = realloc(_enum_pid,_enum_size*64);
	      memset((_enum_pid + (i*64)), 0, 64);
	    }
	    strcpy(_enum_pid + (i*64),entry->d_name);
	    i++;
	  }
	}
      }
    }
    closedir(dir);
    *list = _enum_pid;
    return _enum_size;
  }
  else { return -1; }
}


/* ---------------------------------------------------------------------------*/
/*                       end of metricUnixProcess.c                           */
/* ---------------------------------------------------------------------------*/
