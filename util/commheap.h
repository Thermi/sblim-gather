/*
 * $Id: commheap.h,v 1.1 2004/10/20 08:15:20 mihajlov Exp $
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
 * Description: Memory Management functions for Communcation Functions
 */

#ifndef COMMHEAP_H
#define COMMHEAP_H

#include <stddef.h>

typedef void* COMMHEAP;

COMMHEAP ch_init();
int   ch_release(COMMHEAP ch);
void* ch_alloc(COMMHEAP ch,size_t sz);

#endif
