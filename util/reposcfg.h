/*
 * $Id: reposcfg.h,v 1.1 2004/10/20 14:43:31 heidineu Exp $
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

#ifndef REPOSCFG_H
#define REPOSCFG_H

#include <stddef.h>

int reposcfg_init();
int reposcfg_getitem(const char * key, char * value, size_t maxlen);

#endif
