/*
 * $Id: commutil.c,v 1.2 2004/11/30 13:16:50 mihajlov Exp $
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


#include <commutil.h>

#ifndef AIX
uint64_t htonll(uint64_t v)
{
#if BYTE_ORDER == BIG_ENDIAN
  return v;
#elif BYTE_ORDER == LITTLE_ENDIAN
  return __bswap_64 (v);
#else
# error "What kind of system is this?"
#endif
}
#endif

#ifndef AIX
uint64_t ntohll(uint64_t v)
{
  return htonll(v);
}
#endif

float htonf(float v) 
{
  unsigned char *bp = (unsigned char *)&v;
  unsigned char array[sizeof(float)];

#if BYTE_ORDER == BIG_ENDIAN
  return v;
#elif BYTE_ORDER == LITTLE_ENDIAN
  array[0] = bp[3];
  array[1] = bp[2];
  array[2] = bp[1];
  array[3] = bp[0];
  return (*(float *)array);
#else
# error "What kind of system is this?"
#endif 
}

float ntohf(float v) 
{
  return htonf(v);
}