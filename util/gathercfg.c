/*
 * $Id: gathercfg.c,v 1.1 2004/10/20 08:15:20 mihajlov Exp $
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

static int gathercfg_handle;

int gathercfg_init()
{
  const char * allowed[] = {
    "RepositoryHost",
    "RepositoryPort",
    NULL
  };
  gathercfg_handle=set_configfile("/etc/gatherd.conf",allowed);
  return gathercfg_handle > 0 ? 0 : 1;
}

int gathercfg_getitem(const char * key, char * value, size_t maxlen)
{
  return get_configitem(gathercfg_handle, key, value, maxlen);
}
