/*
 * $Id: lparutil.h,v 1.2 2006/10/27 08:11:12 mihajlov Exp $
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
 * Author:       Viktor Mihajlovski <mihajlov@de.ibm.com>
 * Contributors: 
 *
 * Description:
 * Utility Functions for CEC and LPAR Metric Plugins
 *
 */

/* ---------------------------------------------------------------------------*/

#ifndef LPARUTIL_H
#define LPARUTIL_H

#include <pthread.h>
#include "hypfs.h"

extern pthread_mutex_t  datamutex;
extern HypSystem       *systemdata;
extern HypSystem       *systemdata_old;

#define CPU_MGMTTIME   0
#define CPU_ONLINETIME 1
#define CPU_TIME       2

unsigned long long cpusum(int cputime, HypSystem *system);
void refreshSystemData();

#endif
