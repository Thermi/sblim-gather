/*
 * $Id: metricLocalFileSystem.c,v 1.1 2003/10/17 13:56:01 mihajlov Exp $
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
 * following File System specific metrics :
 *
 * AvailableSpace
 * AvailableSpacePercentage
 *
 */

/* ---------------------------------------------------------------------------*/

#include <mplugin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>       
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>       
#include <mntent.h>
#include <sys/vfs.h>

/* ---------------------------------------------------------------------------*/

static MetricDefinition  metricDef[2];

static ResourceLister          resourceLister;
static ResourceListDeallocator resourceListDeallocator;

/* --- AvailableSpace --- */
static MetricRetriever   metricRetrAvSpace;
static MetricCalculator  metricCalcAvSpace;

/* --- AvailableSpacePercentage --- */
static MetricRetriever   metricRetrAvSpacePerc;
static MetricCalculator  metricCalcAvSpacePerc;

/* ---------------------------------------------------------------------------*/

static int sampleInterval = 60;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/* ---------------------------------------------------------------------------*/

#define ETC_MTAB    "/etc/mtab"
#define PROC_MOUNTS "/proc/mounts"

static char * _enum_fsname = NULL;
static char * _enum_fsdir  = NULL;
static int    _enum_fssize = 0;

static int enum_all_fs();
static int check_enum_fs();

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

  metricDef[0].mdName="AvailableSpace";
  metricDef[0].mdId=mr(pluginname,metricDef[0].mdName);
  metricDef[0].mdSampleInterval=sampleInterval;
  metricDef[0].mdMetricType=MD_RETRIEVED|MD_POINT;
  metricDef[0].mdDataType=MD_UINT64;
  metricDef[0].mproc=metricRetrAvSpace;
  metricDef[0].mdeal=free;
  metricDef[0].mcalc=metricCalcAvSpace;
  metricDef[0].mresl=resourceLister;
  metricDef[0].mresldeal=resourceListDeallocator;

  metricDef[1].mdName="AvailableSpacePercentage";
  metricDef[1].mdId=mr(pluginname,metricDef[1].mdName);
  metricDef[1].mdSampleInterval=sampleInterval;
  metricDef[1].mdMetricType=MD_RETRIEVED|MD_POINT;
  metricDef[1].mdDataType=MD_UINT8;
  metricDef[1].mproc=metricRetrAvSpacePerc;
  metricDef[1].mdeal=free;
  metricDef[1].mcalc=metricCalcAvSpacePerc;
  metricDef[1].mresl=metricDef[0].mresl;
  metricDef[1].mresldeal=metricDef[0].mresldeal;

  *mdnum=2;
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
    fprintf(stderr,"--- %s(%i) : enumerate file systems\n",
	    __FILE__,__LINE__);
#endif    
    if ( (enum_all_fs()!=0) || (_enum_fsname==NULL) ) { 
#ifdef DEBUG
      fprintf(stderr,"--- %s(%i) : enumerate file systems failed\n",
	      __FILE__,__LINE__);
#endif
      return -1; 
    }
  }
  else {
#ifdef DEBUG
    fprintf(stderr,"--- %s(%i) : free file system entries\n",
	    __FILE__,__LINE__);
#endif
    if(_enum_fsname) free(_enum_fsname);
    if(_enum_fsdir)  free(_enum_fsdir);
  }
  
  return 0;
}

int resourceLister(int mid, char *** list) {
  int i = 0;
  if (list) { 
    *list = calloc(_enum_fssize+1,sizeof(char*));
    for(;i<_enum_fssize;i++) {
      (*list)[i] = strdup(_enum_fsname + (i*128));
    }
    return _enum_fssize;
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

int metricRetrAvSpace( int mid, 
		       MetricReturner mret ) {  
  MetricValue   * mv       = NULL;
  struct statfs * fs       = NULL; 
  char          * ptr_name = NULL;
  char          * ptr_dir  = NULL;
  int             i        = 0;
  unsigned long long size  = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Retrieving AvailableSpace\n",
	  __FILE__,__LINE__);
#endif  
  if (mret==NULL) { fprintf(stderr,"Returner pointer is NULL\n"); }
  else {
        
    if( check_enum_fs() != 0) {   
#ifdef DEBUG
      fprintf(stderr,"--- %s(%i) : check internal list of file systems failed\n",
	      __FILE__,__LINE__);
#endif
      return -1; 
    } 

#ifdef DEBUG
    fprintf(stderr,"--- %s(%i) : Sampling for metric AvailableSpace ID %d\n",
	    __FILE__,__LINE__,mid);
#endif
    for(;i<_enum_fssize;i++) {
      
      ptr_name = _enum_fsname + (i*128);
      ptr_dir = _enum_fsdir + (i*128);
      
      fs = (struct statfs *) malloc (sizeof (struct statfs));
      memset(fs, 0, sizeof (struct statfs) );
      if (statfs(ptr_dir, fs) == 0) {
	size = ((unsigned long long)fs->f_bavail) *
	       ((unsigned long long)fs->f_bsize);
      }
      if(fs) free(fs);

      //fprintf(stderr,"[%i] ptr_name: %s ... ptr_dir: %s ... size : %lld\n",i,ptr_name,ptr_dir,size);
      
      mv = calloc(1, sizeof(MetricValue) + 
		     sizeof(unsigned long long) + 
		     (strlen(ptr_name)+1) );
      if (mv) {
	mv->mvId = mid;
	mv->mvTimeStamp = time(NULL);
	mv->mvDataType = MD_UINT64;
	mv->mvDataLength = sizeof(unsigned long long);
	mv->mvData = (void*)mv + sizeof(MetricValue);
	*(unsigned long long*)mv->mvData = size;
	mv->mvResource = (void*)mv + sizeof(MetricValue) + sizeof(unsigned long long);
	strcpy(mv->mvResource,ptr_name);
	mret(mv);
      } 
    }
    return _enum_fssize;
  }
  return -1;
}


size_t metricCalcAvSpace( MetricValue *mv,   
			  int mnum,
			  void *v, 
			  size_t vlen ) {

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate AvailableSpace\n",
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


int metricRetrAvSpacePerc( int mid, 
			   MetricReturner mret ) {  
  MetricValue   * mv       = NULL;
  struct statfs * fs       = NULL; 
  char          * ptr_name = NULL;
  char          * ptr_dir  = NULL;
  int             i        = 0;
  unsigned char   size     = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Retrieving AvailableSpacePercentage\n",
	  __FILE__,__LINE__);
#endif  
  if (mret==NULL) { fprintf(stderr,"Returner pointer is NULL\n"); }
  else {
    
    if( check_enum_fs() != 0) {   
#ifdef DEBUG
      fprintf(stderr,"--- %s(%i) : check internal list of file systems failed\n",
	      __FILE__,__LINE__);
#endif
      return -1; 
    } 

#ifdef DEBUG
    fprintf(stderr,"--- %s(%i) : Sampling for metric AvailableSpacePercentage ID %d\n",
	    __FILE__,__LINE__,mid);
#endif    
    for(;i<_enum_fssize;i++) {
      
      ptr_name = _enum_fsname + (i*128);
      ptr_dir = _enum_fsdir + (i*128);
      
      fs = (struct statfs *) malloc (sizeof (struct statfs));
      memset(fs, 0, sizeof (struct statfs) );
      if (statfs(ptr_dir, fs) == 0) {
	if( fs->f_blocks != 0 && fs->f_bfree != 0 ) {
	  size = ( ( (unsigned long long)fs->f_blocks - 
		     (unsigned long long)fs->f_bfree   ) * 100 ) / 
	         (unsigned long long)fs->f_blocks;
	}
      }
      if(fs) free(fs);

      //fprintf(stderr,"[%i] ptr_name: %s ... ptr_dir: %s ... size : %i\n",i,ptr_name,ptr_dir,size);
      
      mv = calloc(1, sizeof(MetricValue) + 
		     sizeof(unsigned char) + 
		     (strlen(ptr_name)+1) );
      if (mv) {
	mv->mvId = mid;
	mv->mvTimeStamp = time(NULL);
	mv->mvDataType = MD_UINT8;
	mv->mvDataLength = sizeof(unsigned char);
	mv->mvData = (void*)mv + sizeof(MetricValue);
	*(unsigned char*)mv->mvData = size;
	mv->mvResource = (void*)mv + sizeof(MetricValue) + sizeof(unsigned char);
	strcpy(mv->mvResource,ptr_name);
	mret(mv);
      } 
    }
    return _enum_fssize;
  }
  return -1;
}


size_t metricCalcAvSpacePerc( MetricValue *mv,   
			      int mnum,
			      void *v, 
			      size_t vlen ) {

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Calculate AvailableSpacePercentage\n",
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
// get all file system instances, independent of their type
/* ---------------------------------------------------------------------------*/

int enum_all_fs() {  

  struct mntent * sptr = NULL;
  FILE * fhd           = NULL;
  int    i             = 0;  


  if (pthread_mutex_lock(&mutex)==0) {

#ifdef DEBUG
    fprintf(stderr,"--- %s(%i) : enum_all_fs()\n",
	    __FILE__,__LINE__);
#endif
    /* search in mtab for mounted file system entries */
    
    if( (fhd = setmntent( ETC_MTAB ,"r")) == NULL ) {
      fhd = setmntent( PROC_MOUNTS ,"r");
      if ( fhd == NULL ) { return -2; }
    }

    _enum_fssize = 1;
    _enum_fsname = calloc(_enum_fssize,128);
    _enum_fsdir  = calloc(_enum_fssize,128);
    
    while ( ( sptr = getmntent(fhd)) != NULL ) {
      
      if( strcmp(sptr->mnt_fsname,"none") != 0 &&
	  strcmp(sptr->mnt_fsname,"usbdevfs") != 0 ) {
	
	if( i==_enum_fssize ) {
	  _enum_fssize++;
	  _enum_fsname = realloc(_enum_fsname,_enum_fssize*128);
	  memset((_enum_fsname + (i*128)), 0, 128);
	  _enum_fsdir = realloc(_enum_fsdir,_enum_fssize*128);
	  memset((_enum_fsdir + (i*128)), 0, 128);
	}
	
	strcpy(_enum_fsname + (i*128),sptr->mnt_fsname);
	strcpy(_enum_fsdir + (i*128),sptr->mnt_dir);
	i++;
      }
    }
    endmntent(fhd); 
    
    pthread_mutex_unlock(&mutex);
  }

  return 0;
}


int check_enum_fs() {  

  struct stat st;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : check_enum_fs()\n",
	  __FILE__,__LINE__);
#endif
  /* check if ETC_MTAB was changed within the last sample interval */
  if( stat(ETC_MTAB,&st) == 0 ) {
    if( st.st_mtime > (time(NULL)-sampleInterval) ) {
      return enum_all_fs();
    }
    return 0;
  }
  else { return -1; }
}



/* ---------------------------------------------------------------------------*/
/*                    end of metricLocalFileSystem.c                          */
/* ---------------------------------------------------------------------------*/
