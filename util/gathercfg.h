/*
 * $Id: gathercfg.h,v 1.1 2004/10/20 08:15:20 mihajlov Exp $
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

#ifndef GATHERCFG_H
#define GATHERCFG_H

#include <stddef.h>

int gathercfg_init();
int gathercfg_getitem(const char * key, char * value, size_t maxlen);

#endif
