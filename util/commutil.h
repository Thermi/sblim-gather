/*
 * $Id: commutil.h,v 1.2 2004/11/30 13:16:51 mihajlov Exp $
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
 * Author:       Heidi Neumann <heidineu@de.ibm.cim>
 * Contributors: Michael Schuele <schuelem@de.ibm.com>
 *
 * Description: Communcation Utility Functions
 */

#ifndef COMMUTIL_H
#define COMMUTIL_H

#include <netinet/in.h>

#ifndef AIX
uint64_t htonll(uint64_t v);
uint64_t ntohll(uint64_t v);
#endif

float htonf(float v);
float ntohf(float v);

#endif