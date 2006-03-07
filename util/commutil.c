/*
 * $Id: commutil.c,v 1.4 2006/03/07 12:55:21 mihajlov Exp $
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
#include "config.h"

#ifndef AIX
uint64_t htonll(uint64_t v)
{
#ifdef WORDS_BIGENDIAN
  return v;
#else
  return __bswap_64 (v);
#endif
}
#endif

#ifndef AIX
uint64_t ntohll(uint64_t v)
{
  return htonll(v);
}
#endif


/*
 * Note: Don't think this will work in all environments (s390,power)
 */
float htonf(float v) 
{
#ifdef WORDS_BIGENDIAN
  return v;
#else
  unsigned char *bp = (unsigned char *)&v;
  unsigned char array[sizeof(float)];
  int           i;

  for (i=0; i<sizeof(float);i++) {
    array[i] = bp[sizeof(float)-1-i];
  }
  return (*(float *)array);
#endif 
}

float ntohf(float v) 
{
  return htonf(v);
}
