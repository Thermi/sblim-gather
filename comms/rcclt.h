/*
 * $Id: rcclt.h,v 1.2 2004/10/22 12:11:20 heidineu Exp $
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
 * Contributors: Heidi Neumann <heidineu@de.ibm.com>
 *
 * Description: Communication Client APIs
 *              TCP/IP Socket version
 *
 */

#ifndef RCCLT_H
#define RCCLT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include "mcdefs.h"

int rcc_init(const char *srvid, const int *portid);
int rcc_term();
int rcc_request(void *reqdata, size_t reqdatalen);

#ifdef __cplusplus
}
#endif

#endif
