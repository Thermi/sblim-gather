/*
 * $Id: marshal.h,v 1.1 2004/11/12 16:40:12 mihajlov Exp $
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
 * Description: Marshalling
 */

#ifndef MARSHAL_H
#define MARSHAL_H

#include "repos.h"

#ifdef __cplusplus
extern "C" {
#endif

int marshal_string(const char *str, char *mbuf, off_t *offset, size_t mbuflen, 
		   int required);
int unmarshal_string(char **str, const char *mbuf, off_t *offset, 
		     size_t mbuflen, int required);

int marshal_data(const void *data, size_t datalen, char *mbuf, 
		 off_t *offset, size_t mbuflen);
int unmarshal_data(void **data, size_t datalen, const char *mbuf, 
		   off_t *offset, size_t mbuflen);

int marshal_valuerequest(const ValueRequest *vr, char *mbuf, 
			 off_t * offset, size_t mbuflen);
int unmarshal_valuerequest(ValueRequest **vr, const char *mbuf, 
			   off_t * offset, size_t mbuflen);

int marshal_subscriptionrequest(const SubscriptionRequest *vr, char *mbuf,
				off_t * offset, size_t mbuflen);
int unmarshal_subscriptionrequest(SubscriptionRequest **vr, const char *mbuf, 
				  off_t * offset, size_t mbuflen);

#ifdef __cplusplus
}
#endif

#endif
