/*
 * $Id: mcdefs.h,v 1.2 2004/07/16 15:30:05 mihajlov Exp $ 
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
 * Description: Communication API Defintions
 */

#ifndef MCDEFS_H
#define MCDEFS_H

#ifdef __cplusplus
extern "C" {
#endif
  
#define MC_SOCKET  "/tmp/.%s-socket"
#define MC_LOCKFILE  "/tmp/.%s-lockfile"
#define MC_IPCFILE "/tmp/.%s-ipc"
#define MC_IPCPROJ 100

struct _MC_REQHDR
{
  int            mc_handle;
  unsigned short mc_type;

};

typedef struct _MC_REQHDR MC_REQHDR;

#ifdef __cplusplus
}
#endif

#endif
