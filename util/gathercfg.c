/*
 * $Id: gathercfg.c,v 1.4 2006/02/08 20:26:46 mihajlov Exp $
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
 * Author:       Viktor Mihajlovski <mihajlov@de.ibm.cim>
 * Contributors: 
 *
 * Description: Gatherd Configuration
 *
 */

#include "gathercfg.h"
#include "mcfg.h"

#ifndef GATHER_CONFDIR
#define GATHER_CONFDIR "/etc"
#endif

static int gathercfg_handle;

int gathercfg_init()
{
  const char * allowed[] = {
    "RepositoryHost",
    "RepositoryPort",
    "TraceLevel",
    "TraceFile",
    "TraceComponents",
    "PluginDirectory",
    "AutoLoad",
    NULL
  };
  gathercfg_handle=set_configfile(GATHER_CONFDIR "/gatherd.conf",allowed);
  return gathercfg_handle > 0 ? 0 : 1;
}

int gathercfg_getitem(const char * key, char * value, size_t maxlen)
{
  return get_configitem(gathercfg_handle, key, value, maxlen);
}
