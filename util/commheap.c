/*
 * $Id: commheap.c,v 1.1 2004/10/20 08:15:20 mihajlov Exp $
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
 * Description: Memory Management for Communication Functions.
 */

#include "commheap.h"
#include <malloc.h>
#include <stdio.h>

struct _COMMHEAP_CTL {
  int   cc_max;
  int   cc_index;
  void **cc_arr; 
};

typedef struct _COMMHEAP_CTL COMMHEAP_CTL;

COMMHEAP ch_init()
{
  return calloc(1,sizeof(COMMHEAP_CTL));
}

void* ch_alloc(COMMHEAP ch, size_t sz)
{
  COMMHEAP_CTL * cc = (COMMHEAP_CTL*)ch;
  if (cc && cc->cc_index < cc->cc_max - 1) {
    return (cc->cc_arr[++cc->cc_index] = malloc(sz));
  } else {
    cc->cc_max+=1000;
    cc->cc_arr=realloc(cc->cc_arr,(cc->cc_max)*sizeof(void*));
    return ch_alloc(ch,sz);
  }
}

int ch_release(COMMHEAP ch)
{
  COMMHEAP_CTL * cc = (COMMHEAP_CTL*)ch;
  if (cc) {
    while (cc->cc_index > 0) free (cc->cc_arr[cc->cc_index--]);
    if (cc->cc_arr) free (cc->cc_arr);
    free (cc);
    return 0;
  } else {
    return -1;
  }
}
