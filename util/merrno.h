/*
 * $Id: merrno.h,v 1.1 2004/10/21 15:51:18 mihajlov Exp $
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
 * Description: Error handling support
 *
 */

#ifndef MERRNO_H
#define MERRNO_H

#include <merrdefs.h>

int m_errno();
void m_seterrno(int err);
char * m_strerror();
void m_setstrerror(const char *fmt, ...);

#endif
