/*
 * $Id: metricNetworkPort.c,v 1.3 2004/09/15 11:29:40 heidineu Exp $
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
 * Metrics Gatherer Plugin of the following Network Port specific metrics :
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
#include <time.h>

/* ---------------------------------------------------------------------------*/

static MetricDefinition  metricDef[1];

/* --- BytesSubmitted is base for :
 * BytesTransmitted, BytesReceived, ErrorRate --- */
static MetricRetriever   metricRetrBytesSubmitted;

/* ---------------------------------------------------------------------------*/

#define NETINFO "/proc/net/dev"

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
  metricDef[0].mdName="BytesSubmitted";
  metricDef[0].mdReposPluginName="librepositoryNetworkPort.so";
  metricDef[0].mdId=mr(pluginname,metricDef[0].mdName);
  metricDef[0].mdSampleInterval=60;
  metricDef[0].mproc=metricRetrBytesSubmitted;
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

int metricRetrBytesSubmitted( int mid, 
			      MetricReturner mret ) {  
  MetricValue * mv  = NULL;
  FILE * fhd        = NULL;
  char * ptr        = NULL;
  char * end        = NULL;
  char * col        = NULL;
  char   port[64];
  char   buf[60000];
  char   values[(6*sizeof(unsigned long long))+6];
  size_t bytes_read = 0;
  int    i          = 0;
  unsigned long long receive_byte, receive_packets, receive_error = 0;
  unsigned long long trans_byte, trans_packets, trans_error       = 0;

#ifdef DEBUG
  fprintf(stderr,"--- %s(%i) : Retrieving BytesSubmitted\n",
	  __FILE__,__LINE__);
#endif  
  if (mret==NULL) { fprintf(stderr,"Returner pointer is NULL\n"); }
  else {

#ifdef DEBUG
    fprintf(stderr,"--- %s(%i) : Sampling for metric BytesSubmitted ID %d\n",
	    __FILE__,__LINE__,mid); 
#endif    

    if( (fhd = fopen(NETINFO,"r")) != NULL ) {
      bytes_read = fread(buf, 1, sizeof(buf)-1, fhd);
      if( bytes_read > 0 ) {
	/* skip first two lines */
	ptr = strchr(buf,'\n')+1;
	ptr = strchr(ptr,'\n')+1;
	  
	while( (end = strchr(ptr,'\n')) != NULL ) {
	  sscanf(ptr,
		 "%s %lld %lld %lld %*s %*s %*s %*s %*s %lld %lld %lld",
		 &port,
		 &receive_byte,&receive_packets,&receive_error,
		 &trans_byte,&trans_packets,&trans_error);
	  
	  col = strchr(port,':');
	  *col = '\0';
	  fprintf(stderr,"[%i] port: %s \n",i,port);
	  
	  memset(values,0,sizeof(values));
	  sprintf(values,"%lld:%lld:%lld:%lld:%lld:%lld",
		  receive_byte,trans_byte,receive_error,trans_error,receive_packets,trans_packets);
	  
	  mv = calloc(1, sizeof(MetricValue) + 
		      (strlen(values)+1) + 
		      (strlen(port)+1) );
	  if (mv) {
	    mv->mvId = mid;
	    mv->mvTimeStamp = time(NULL);
	    mv->mvDataType = MD_STRING;
	    mv->mvDataLength = (strlen(values)+1);
	    mv->mvData = (void*)mv + sizeof(MetricValue);
	    strcpy(mv->mvData,values);
	    mv->mvResource = (void*)mv + sizeof(MetricValue) + (strlen(values)+1);
	    strcpy(mv->mvResource,port);
	    mret(mv);
	  } 
	  ptr = end+1;
	  i++;
	}
       	fclose(fhd);
      }
      else { return -1; }
    }
    return i;
  }
  return -1;
}


/* ---------------------------------------------------------------------------*/
/*                          end of metricNetworkPort.c                        */
/* ---------------------------------------------------------------------------*/
