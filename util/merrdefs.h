/*
 * $Id: merrdefs.h,v 1.1 2004/10/21 15:51:18 mihajlov Exp $
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
 * Description: Error Codes
 *
 */

#ifndef MERRDEFS_H
#define MERRDEFS_H

#define MERRNO_BASE         0
#define MERRNO_BASE_UTIL    (MERRNO_BASE + 100)
#define MERRNO_BASE_COMM    (MERRNO_BASE_UTIL + 100)
#define MERRNO_BASE_GATHER  (MERRNO_BASE_COMM + 100)
#define MERRNO_BASE_REPOS   (MERRNO_BASE_GATHER + 100)

#endif
