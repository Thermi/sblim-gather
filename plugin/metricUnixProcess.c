/*
 * $Id: metricUnixProcess.c,v 1.3 2003/12/05 13:50:04 heidineu Exp $
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
 * following Unix Process specific metrics :
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
#include <time.h>

#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

/* ---------------------------------------------------------------------------*/

static MetricDefinition  metricDef[7];

static ResourceLister          resourceLister;
static ResourceListDeallocator resourceListDeallocator;

/* --- CPUTime is base for :
 * KernelModeTime, UserModeTime, TotalCPUTime --- */
static MetricRetriever   metricRetrCPUTime;
static MetricCalculator  metricCalcCPUTime;

static MetricCalculator  metricCalcKernelTime;
static MetricCalculator  metricCalcUserTime;
static MetricCalculator  metricCalcTotalCPUTime;

/* --- ResidentSetSize --- */
static MetricRetriever   metricRetrResSetSize;
static MetricCalculator  metricCalcResSetSize;

/* --- PageInCounter, PageInRate --- */
static MetricRetriever   metricRetrPageInCounter;
static MetricCalculator  metricCalcPageInCounter;

static MetricCalculator  metricCalcPageInRate;

/* ---------------------------------------------------------------------------*/

static int enum_all_pid();

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

  metricDef[0].mdName="CPUTime";
  metricDef[0].mdId=mr(pluginname,metricDef[0].mdName);
  metricDef[0].mdSampleInterval=60;
  metricDef[0].mdMetricType=MD_RETRIEVED|MD_POINT;
  metricDef[0].mdDataType=MD_STRING;
  metricDef[0].mproc=metricRetrCPUTime;
  metricDef[0].mdeal=free;
  metricDef[0].mcalc=metricCalcCPUTime;
  metricDef[0].mresl=resourceLister;
  metricDef[0].mresldeal=resourceListDeallocator;

  metricDef[1].mdName="KernelModeTime";
  metricDef[1].mdId=mr(pluginname,metricDef[1].mdName);
  metricDef[1].mdMetricType=MD_CALCULATED|MD_INTERVAL;
  metricDef[1].mdDataType=MD_UINT64;
  metricDef[1].mdAliasId=metricDef[0].mdId;
  metricDef[1].mproc=NULL;
  metricDef[1].mdeal=NULL;
  metricDef[1].mcalc=metricCalcKernelTime;
  metricDef[1].mresl=metricDef[0].mresl;
  metricDef[1].mresldeal=metricDef[0].mresldeal;

  metricDef[2].mdName="UserModeTime";
  metricDef[2].mdId=mr(pluginname,metricDef[2].mdName);
  metricDef[2].mdMetricType=MD_CALCULATED|MD_INTERVAL;
  metricDef[2].mdDataType=MD_UINT64;
  metricDef[2].mdAliasId=metricDef[0].mdId;
  metricDef[2].mproc=NULL;
  metricDef[2].mdeal=NULL;
  metricDef[2].mcalc=metricCalcUserTime;
  metricDef[2].mresl=metricDef[0].mresl;
  metricDef[2].mresldeal=metricDef[0].mresldeal;

  metricDef[3].mdName="TotalCPUTime";
  metricDef[3].mdId=mr(pluginname,metricDef[3].mdName);
  metricDef[3].mdMetricType=MD_CALCULATED|MD_INTERVAL;
  metricDef[3].mdDataType=MD_UINT64;
  metricDef[3].mdAliasId=metricDef[0].mdId;
  metricDef[3].mproc=NULL;
  metricDef[3].mdeal=NULL;
  metricDef[3].mcalc=metricCalcTotalCPUTime;
  metricDef[3].mresl=metricDef[0].mresl;
  metricDef[3].mresldeal=metricDef[0].mresldeal;

  metricDef[4].mdName="ResidentSetSize";
  metricDef[4].mdId=mr(pluginname,metricDef[4].mdName);
  metricDef[4].mdSampleInterval=60;
  metricDef[4].mdMetricType=MD_RETRIEVED|MD_POINT;
  metricDef[4].mdDataType=MD_UINT64;
  metricDef[4].mproc=metricRetrResSetSize;
  metricDef[4].mdeal=free;
  metricDef[4].mcalc=metricCalcResSetSize;
  metricDef[4].mresl=metricDef[0].mresl;
  metricDef[4].mresldeal=metricDef[0].mresldeal;

  metricDef[5].mdName="PageInCounter";
  metricDef[5].mdId=mr(pluginname,metricDef[5].mdName);
  metricDef[5].mdSampleInterval=60;
  metricDef[5].mdMetricType=MD_RETRIEVED|MD_POINT;
  metricDef[5].mdDataType=MD_UINT64;
  metricDef[5].mproc=metricRetrPageInCounter;
  metricDef[5].mdeal=free;
  metricDef[5].mcalc=metricCalcPageInCounter;
  metricDef[5].mresl=metricDef[0].mresl;
  metricDef[5].mresldeal=metricDef[0].mresldeal;

  metricDef[6].mdName="PageInRate";
  metricDef[6].mdId=mr(pluginname,metricDef[6].mdName);
  metricDef[6].mdMetricType=MD_CALCULATED|MD_RATE;
  metricDef[6].mdDataType=MD_UINT64;
  metricDef[6].mdAliasId=metricDef[5].mdId;
  metricDef[6].mproc=NULL;
  metricDef[6].mdeal=NULL;
  metricDef[6].mcalc=metricCalcPageInRate;
  metricDef[6].mresl=metricDef[0].mresl;
  metricDef[6].mresldeal=metricDef[0].mresldeal;

  *mdnum=7;
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
  char * _enum_pid  = NULL;
  int    _enum_size = 0;
  int    i          = 0;

  _enum_size = enum_all_pid( &_enum_pid );
  if( _enum_size > 0 ) {
    if (list) { 
      *list = calloc(_enum_size+1,sizeof(char*));
      for(;i<_enum_size;i++) {
	(*list)[i] = strdup(_enum_pid + (i*64));
#ifdef DEBUG
	fprintf(stderr,"resourceLister (*list)[i] %s\n",(*list)[i]);
#endif
      }
      if(_enum_pid) free(_enum_pid);
      return _enum_size;
    }
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
 * The raw data CPUTime has the following syntax :
 *
 * <user mode time>:<kernel mode time>
 *
 * the values in CPUTime are saved in Jiffies ( 1/100ths of a second )
 */

int metricRetrCPUTime( int mid, 
		       MetricReturner mret ) {
  MetricValue * mv          = NULL;
  FILE        * fhd         = NULL;
  char        * _enum_pid   = NULL;
  char          buf[254];
  int           _enum_size  = 0;
  int           i           = 0;
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
	sprintf(buf,"%lld:%lld",u_time,k_time);

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
// get all Process IDs 
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
#ifdef DEBUG
	    //fprintf(stderr,"_enum_pid : %s\n",_enum_pid + (i*64));
#endif
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
