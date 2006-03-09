/*
 * $Id: mreposp.c,v 1.5 2006/03/09 15:55:58 mihajlov Exp $
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
 * Author:       Viktor Mihajlovski <mihajlov@de.ibm.cim>
 * Contributors: 
 *
 * Description:  Metric Value Repository Proxy
 * This is a forwarder to a remote repository daemon.
 *
 */

#include "mrepos.h"
#include "mreg.h" 
#include "rrepos.h" 

#include <stdlib.h>
#include <limits.h>
#include <string.h>
  
int ProxyMetricAdd (MetricValue *mv);
int ProxyMetricRetrieve (int mid, MetricResourceId *resource,
			 MetricValue **mv, int *num, 
			 time_t from, time_t to, int maxnum);
int ProxyMetricRelease (MetricValue *mv);
int ProxyShutdown ();

static MetricRepositoryIF mrep = {
  "ProxyRepository",
  ProxyMetricAdd,
  ProxyMetricRetrieve,
  ProxyMetricRelease,
  NULL,
  NULL,
  NULL,
  ProxyShutdown
};

MetricRepositoryIF *MetricRepository = &mrep;

int ProxyMetricAdd (MetricValue *mv)
{
  int rc=-1;
#ifdef NAGNAG
  /*
   * 1. Need to verify connection to repository daemon
   * 2. If not established, connect
   * 3. Verify that plugin is registered remotely 
   * 4. If not register plugin
   * 5. send value(s) to remote repository
   *
   */

  COMMHEAP ch = ch_init();
  
  switch (rrepos_sessioncheck()) {
  case 1:
    /* session was (re)established - need to (re)register plugins  */
    {
      PluginDefinition *pdef;
      char ** plugins;
      metricpluginname_list(&plugins,ch);
      while (*plugins) {
	metricplugin_list(*plugins,&pdef,ch);
	rrepos_register(*plugins,pdef);
      }
    }
    break;
  case -1:
    ch_release(ch);
    return -1;
    /* todo: log failure */
    break;
  }
#endif
  
  MetricDefinition *md=MPR_GetMetric(mv->mvId);
  rc=rrepos_put(md->mdReposPluginName,md->mdName,mv);
  if (md->mdeal) md->mdeal(mv);
  return rc;
}

/*
 * Retrieves metric values for metric id mid,
 * a resource (if specified) in a given time range.
 * The MetricValue array must be freed by a call 
 * to ProxyMetricRelease.
 *
 * Behavior:
 * I) maxnum <= 0
 *    i) to <= 0: retrieve everything starting with "from"
 *    ii) i > 0: retrieve everything between "from" and "to"
 * II) actnum >0
 *    i) from >= to (!=0) retrieve "maxnum" entries starting with "from"  
 *    ii) from== 0 retrieve the newest "maxnum" entries 
 */

int ProxyMetricRetrieve (int mid, MetricResourceId *resource,
			 MetricValue **mv, int *num, 
			 time_t from, time_t to, int maxnum)
{
  return -1;
}

int ProxyMetricRelease (MetricValue *mv)
{
  return -1;
}

int ProxyShutdown ()
{
  return -1;
}


