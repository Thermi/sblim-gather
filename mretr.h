/*
 * $Id: mretr.h,v 1.1 2003/10/17 13:56:01 mihajlov Exp $
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
 * Description:  Threaded metric retriever.
 * This module offers services to process a metric block lists with
 * multiple threads in parallel.
 *
 */

#ifndef MRETR_H
#define MRETR_H

#include "mlist.h"

typedef void* MR_Handle;

MR_Handle MR_Init(ML_Head mhead, unsigned numthreads);
void MR_Finish(MR_Handle mrh);
void MR_Wakeup(MR_Handle mrh);

#endif
