/*
 * $Id: mlog.c,v 1.1 2004/10/20 08:15:21 mihajlov Exp $
 *
 * (C) Copyright IBM Corp. 2003, 2004
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
 * Description: Metric Defintiona and Value data types.
 *
 */

const char *_mlog_id = "$Id: mlog.c,v 1.1 2004/10/20 08:15:21 mihajlov Exp $";

#include "mlog.h"
#include <syslog.h>
#include <stdarg.h>
#include <stdio.h>

void m_start_logging(const char *name)
{
  openlog(name,LOG_PID,LOG_DAEMON);
  setlogmask(LOG_UPTO(LOG_INFO));
}

void m_log(int priority, int errout, const char *fmt, ...)
{
  va_list ap;
  int priosysl;
  switch (priority) {
  case M_DEBUG:
    priosysl=LOG_DEBUG;
    break;
  case M_INFO:
    priosysl=LOG_INFO;
    break;
  case M_ERROR:
  default:
    priosysl=LOG_ERR;
    break;
  }
  va_start(ap,fmt);
  vsyslog(priosysl,fmt,ap);
  if (errout) {
    vfprintf(stderr,fmt,ap);
  }
}
