/*
 * $Id: mcclt.h,v 1.2 2004/07/16 15:30:05 mihajlov Exp $
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
 * Description: Communication Client APIs
 */

#ifndef MCCLT_H
#define MCCLT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>  
#include "mcdefs.h"

int mcc_init(const char *commid);
int mcc_term();
int mcc_request(MC_REQHDR *hdr, void *reqdata, size_t reqdatalen);
int mcc_response(MC_REQHDR *hdr, void *respdata, size_t *respdatalen);

#ifdef __cplusplus
}
#endif

#endif
