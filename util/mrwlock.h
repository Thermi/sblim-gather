/*
 * $Id: mrwlock.h,v 1.1 2004/10/20 08:15:21 mihajlov Exp $
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
 * Description: Multiple Reader/Single Writer Lock
 * This module facilitates scalable multiple reader single writer
 * processing paradigms.
 *
 */

#ifndef MRWLOCK_H
#define MRWLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

typedef struct _MRWLOCK {
  pthread_mutex_t mrw_mutex;
  pthread_cond_t  mrw_cond;
  unsigned        mrw_rnum;
} MRWLOCK;

/* Macro for stativ MRW Lock Definition */
#define MRWLOCK_DEFINE(n) MRWLOCK n={PTHREAD_MUTEX_INITIALIZER, \
                                     PTHREAD_COND_INITIALIZER, \
                                     0}
int MRWInit(MRWLOCK *mrwl);
int MRWTerm(MRWLOCK *mrwl);
int MReadLock(MRWLOCK *mrwl);
int MReadUnlock(MRWLOCK *mrwl);
int MWriteLock(MRWLOCK *mrwl);
int MWriteUnlock(MRWLOCK *mrwl);

#ifdef __cplusplus
}
#endif

#endif