/*
 * $Id: sforward.h,v 1.2 2004/11/26 15:25:34 mihajlov Exp $
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
 * Description: Subscription Forwarding
 */

#ifndef SFORWARD_H
#define SFORWARD_H

#include "repos.h"

#ifdef __cplusplus
extern "C" {
#endif

int subs_enable_forwarding(SubscriptionRequest *sr, const char *listenerid);
int subs_disable_forwarding(SubscriptionRequest *sr, const char *listenerid);
void subs_forward(int corrid, ValueRequest *vr);

#ifdef __cplusplus
}
#endif

#endif
