/*
 * $Id: mcfg.h,v 1.1 2004/10/19 12:06:56 mihajlov Exp $
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
 * Description: Simple Configuration Support
 *
 */

#ifndef MCFG_H
#define MCFG_H

#include <stddef.h>

int set_configfile(const char * filename, const char ** keys);
int get_configitem(const char * key, char *value, size_t maxlen);

#endif