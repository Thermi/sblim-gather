/*
 * $Id: gatherc.h,v 1.1 2003/10/17 13:56:01 mihajlov Exp $
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
 * Description:  Gatherer Communications Definition
 *
 */

#ifndef GATHERC_H
#define GATHERC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

#define GATHERMC_REQ     1
#define GATHERMC_RESP    2 

#define GCMD_INIT        1
#define GCMD_TERM        2
#define GCMD_START       3
#define GCMD_STOP        4
#define GCMD_STATUS      5
#define GCMD_ADDPLUGIN   6
#define GCMD_REMPLUGIN   7
#define GCMD_GETVALUE    8
#define GCMD_LISTPLUGIN  9

#define GCMD_QUIT       99

#define GATHERBUFLEN    1000
#define GATHERVALBUFLEN 1000000

typedef struct _GATHERCOMM {
  short  gc_cmd;
  short  gc_result;
  size_t gc_datalen;
} GATHERCOMM;

#ifdef __cplusplus
}
#endif

#endif
