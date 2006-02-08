/*
 * $Id: reposcfg.c,v 1.4 2006/02/08 20:26:46 mihajlov Exp $
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
 * Description: Reposd Configuration
 *
 */

#include "reposcfg.h"
#include "mcfg.h"

#ifndef GATHER_CONFDIR
#define GATHER_CONFDIR "/etc"
#endif

static int reposcfg_handle;

int reposcfg_init()
{
  const char * allowed[] = {
    "RepositoryPort",
    "RepositoryMaxConnections",
    "TraceLevel",
    "TraceFile",
    "TraceComponents",
    "PluginDirectory",
    "AutoLoad",
    NULL
  };
  reposcfg_handle=set_configfile(GATHER_CONFDIR "/reposd.conf",allowed);
  return reposcfg_handle > 0 ? 0 : 1;
}

int reposcfg_getitem(const char * key, char * value, size_t maxlen)
{
  return get_configitem(reposcfg_handle, key, value, maxlen);
}
