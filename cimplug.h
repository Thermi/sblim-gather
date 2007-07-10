/*
 * $Id: cimplug.h,v 1.3 2007/07/10 13:23:28 mihajlov Exp $
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
 * Description: CIM Naming Plugin Definition
 *
 */

#ifndef CIMPLUG_H
#define CIMPLUG_H

#include <cmpidt.h>
#include <cmpift.h>
#include <cmpimacs.h>

#include <stddef.h>

/* entry point names */

#define COP4VALID                _CopFromValId
#define VALID4COP                _ValIdFromCop
#define GETRES                   _GetResources
#define FREERES                  _FreeResources

#define COP4VALID_S              "_CopFromValId"
#define VALID4COP_S              "_ValIdFromCop"
#define GETRES_S                 "_GetResources"
#define FREERES_S                "_FreeResources"

/* 
 * construct a namespace less object path from a metric definition or
 * metric value resource id. Note: the system id can be used for heuristically
 * setting some of the key values (like system name). Handle with care!
 * Return NULL in case of failure.
 */

typedef CMPIObjectPath* (CimObjectPathFromValueId)
     (CMPIBroker *broker, const char* id, const char* systemid);

/* 
 * construct a metric definition or metric value id from an object path.
 * Note: if the system id can be derived from the object path it should
 * be set by the plugin.
 * Return -1 in case of failure, 0 if successful 
 */

typedef int (ValueIdFromCimObjectPath)
     (CMPIObjectPath* cop, 
      char* id, size_t idlen, char* systemid, size_t systemidlen);


/*
 * return list of resource classes handled by this plugin
 */

typedef int (GetResourceClasses)(char ***);

/*
 * free list of resource classes returned by GetResourceClasses
 */

typedef void (FreeResourceClasses)(char **);


#endif
