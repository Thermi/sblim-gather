/*
 * $Id: metricProcessor.c,v 1.2 2004/08/04 09:00:04 heidineu Exp $
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
 * Metrics Gatherer Plugin of the following Processor specific metrics :
 *
 * TotalCPUTimePercentage
 *
 */

/* ---------------------------------------------------------------------------*/

#include <mplugin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     
#include <time.h>
#include <unistd.h>

/* ---------------------------------------------------------------------------*/

static MetricDefinition metricDef[1];

/* --- TotalCPUTimePercentage --- */
static MetricRetriever  metricRetrCPUTimePerc;

/* ---------------------------------------------------------------------------*/

#define CPUINFO "/proc/cpuinfo"
//#define CPUINFO "/home/heidineu/sblim/src/cmpi-base-cpuinfo/2x86_ibm_xSeries"
//#define CPUINFO "/home/heidineu/sblim/src/cmpi-base-cpuinfo/4x86_ibm_xSeries"
//#define CPUINFO "/home/heidineu/sblim/src/cmpi-base-cpuinfo/power_ibm_iSeries"
//#define CPUINFO "/home/heidineu/sblim/src/cmpi-base-cpuinfo/power_ibm_iSeries_2x"
//#define CPUINFO "/home/heidineu/sblim/src/cmpi-base-cpuinfo/power_ibm_pSeries"
//#define CPUINFO "/home/heidineu/sblim/src/cmpi-base-cpuinfo/power_ibm_pSeries_2x"
//#define CPUINFO "/home/heidineu/sblim/src/cmpi-base-cpuinfo/s390_ibm_s390"
//#define CPUINFO "/home/heidineu/sblim/src/cmpi-base-cpuinfo/s390_ibm_s390_2x"

static char * _enum_proc = NULL;
static int    _enum_size = 0;

static int enum_all_proc();

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
  metricDef[0].mdName="TotalCPUTimePercentage";
  metricDef[0].mdReposPluginName="librepositoryProcessor.so";
  metricDef[0].mdId=mr(pluginname,metricDef[0].mdName);
  metricDef[0].mdSampleInterval=60;
  metricDef[0].mproc=metricRetrCPUTimePerc;
  metricDef[0].mdeal=free;

  *mdnum=1;
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
    fprintf(stderr,"--- %s(%i) : enumerate processors\n",
	    __FILE__,__LINE__);
#endif
    if( enum_all_proc() != 0 ) { return -1; }
  }
  else {
#ifdef DEBUG
    fprintf(stderr,"--- %s(%i) : free processor entries\n",
	    __FILE__,__LINE__);
#endif
    if(_enum_proc) free(_enum_proc);
  }
  
  return 0;
}


/* ---------------------------------------------------------------------------*/
/* TotalCPUTimePercentage                                                     */
/* ---------------------------------------------------------------------------*/

int metricRetrCPUTimePerc( int mid, 
			   MetricReturner mret ) {  
  MetricValue * mv  = NULL;
  FILE * fhd        = NULL;
  char * ptr        = NULL;
  char * end        = NULL;
  char * proc       = NULL;
  char   buf[60000];
  size_t bytes_read = 0;
  int    i          = 0;
  unsigned char      size = 0;
  unsigned long long val1 = 0;
  unsigned long long val2 = 0;
  unsigned long long val3 = 0;
  unsigned long long val4 = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Retrieving TotalCPUTimePercentage\n",
	  __FILE__,__LINE__);
#endif  
  if (mret==NULL) { fprintf(stderr,"Returner pointer is NULL\n"); }
  else {

#ifdef DEBUG
    fprintf(stderr,"--- %s(%i) : Sampling for metric TotalCPUTimePercentage ID %d\n",
	    __FILE__,__LINE__,mid); 
#endif
    if( (fhd = fopen("/proc/stat","r")) != NULL ) {

      bytes_read = fread(buf, 1, sizeof(buf)-1, fhd);
      if( bytes_read > 0 ) {
	for(;i<_enum_size;i++) {
	  ptr = strchr(buf,'\n')+1;
	  sscanf(ptr,"%*s %lld %lld %lld %lld",&val1,&val2,&val3,&val4);
	  size = ((val1+val2+val3)*100) / (val1+val2+val3+val4);
	  proc = _enum_proc + (i*64);	  
	  //fprintf(stderr,"[%i] proc: %s ... size : %i\n",i,proc,size);

	  mv = calloc(1, sizeof(MetricValue) + 
		         sizeof(unsigned char) + 
		         (strlen(proc)+1) );
	  if (mv) {
	    mv->mvId = mid;
	    mv->mvTimeStamp = time(NULL);
	    mv->mvDataType = MD_UINT8;
	    mv->mvDataLength = sizeof(unsigned char);
	    mv->mvData = (void*)mv + sizeof(MetricValue);
	    *(unsigned char*)mv->mvData = size;
	    mv->mvResource = (void*)mv + sizeof(MetricValue) + sizeof(unsigned char);
	    strcpy(mv->mvResource,proc);
	    mret(mv);
	  }  
	  ptr = end+1;
	}
      }
      fclose(fhd);
      return _enum_size;
    }
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/
/* get all processor instances                                                */
/* ---------------------------------------------------------------------------*/

int enum_all_proc() {  

  char * ptr        = NULL;
  char * str        = NULL;
  char * proc       = NULL;
  char * end        = NULL;
  char * id         = NULL;
  char * cmd        = NULL;
  char   buf[60000];
  size_t bytes_read = 0;
  int    i          = 0;
  int    fd_out[2]; 
  int    fd_stdout;
  int    fd_err[2]; 
  int    fd_stderr;
  int    rc         = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : enum_all_proc()\n",__FILE__,__LINE__);
#endif
  /* search for processor entries in CPUINFO */

  if( pipe(fd_out)==0 && pipe(fd_err)==0 ) {

    fd_stdout = dup( fileno(stdout) );
    close(fileno(stdout));
    dup2( fd_out[1], fileno(stdout) );

    fd_stderr = dup( fileno(stderr) );
    close(fileno(stderr));
    dup2( fd_err[1], fileno(stderr) );

    cmd = calloc(1,(strlen(CPUINFO)+46));
    strcpy(cmd, "cat ");
    strcat(cmd, CPUINFO);
    strcat(cmd, " | grep ^processor | sed -e s/processor//");
    
    rc = system(cmd);
    if( rc == 0 ) { bytes_read = read( fd_out[0], buf, sizeof(buf)-1 ); }
    else          { bytes_read = read( fd_err[0], buf, sizeof(buf)-1 ); }
    
    close(fd_out[1]);
    dup2( fd_stdout, fileno(stdout) );
    close(fd_out[0]);
    close(fd_stdout);
    
    close(fd_err[1]);
    dup2( fd_stderr, fileno(stderr) );
    close(fd_err[0]);
    close(fd_stderr);
    
    if(cmd) free(cmd);
  }
  else { return -1; }

  if( bytes_read > 0 ) {
    ptr = buf;
    while( (str = strchr(ptr,'\n')) != NULL ) {
      ptr = str+1;
      i++; 
    }
  }

  if( i > 0 ) {
    _enum_size = i;
    _enum_proc = calloc(_enum_size,64);
   
    i=0;
    ptr = buf;
    for(;i<_enum_size;i++) {
      str = strchr(ptr,'\n'); 
      ptr = strchr(ptr,':');
      
#if defined (INTEL) || defined (PPC)
      while( (ptr=strchr(ptr,' ')) != NULL) { 
	if( ptr > str) { break; }
	id = ptr+1; 
	ptr++;
      }
      end = str;
#elif defined (S390)
      id = ptr-1;
      end = ptr;
#endif
      
      proc = _enum_proc + (i*64);
      strncpy(proc, id, strlen(id)-strlen(end));      
      ptr = str+1;
    }
  }

  return 0;
}

/* ---------------------------------------------------------------------------*/
/*                          end of metricProcessor.c                          */
/* ---------------------------------------------------------------------------*/
