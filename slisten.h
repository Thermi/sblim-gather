/*
 * $Id: slisten.h,v 1.1 2004/11/12 16:40:12 mihajlov Exp $
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
 * Description: Subscription Listening
 */

#ifndef SLISTEN_H
#define SLISTEN_H

#include "repos.h"

#ifdef __cplusplus
extern "C" {
#endif

int add_subscription_listener(char *listenerid, SubscriptionRequest *sr,
			      SubscriptionCallback * scb);

#ifdef __cplusplus
}
#endif

#endif
