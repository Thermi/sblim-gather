/*
 * $Id: cimplugIPProtocolEndpoint.c,v 1.1 2004/10/08 07:34:54 mihajlov Exp $
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
 * Author:       Heidi Neumann <heidineu@de.ibm.com>
 * Contributors: 
 *
 * Description:
 * CIM Id Plugin for IP Protocol Endpoint specific metrics
 */

#include <cimplug.h>
#include <string.h>
#include <stdlib.h>

CMPIObjectPath* COP4VALID (CMPIBroker *broker, const char *id, 
			const char *systemid)
{
  CMPIObjectPath *cop = CMNewObjectPath(broker,NULL,"Linux_IPProtocolEndpoint",
					NULL);
  if (cop) {
    CMAddKey(cop,"Name",id,CMPI_chars);
    CMAddKey(cop,"CreationClassName","Linux_IPProtocolEndpoint",CMPI_chars);
    CMAddKey(cop,"SystemName",systemid,CMPI_chars);
    CMAddKey(cop,"SystemCreationClassName","Linux_ComputerSystem",
	     CMPI_chars);
  }
  return cop;
}

int VALID4COP (const CMPIObjectPath *cop, char *id, size_t idlen,
	    char *systemid, size_t systemidlen)
{
  CMPIData data;
  char    *str;
  
  if (cop && id && systemid) {
    data=CMGetKey(cop,"Name",NULL);
    if (data.type==CMPI_string && data.value.string) {
      str=CMGetCharPtr(data.value.string);
      if (strlen(str)<idlen) {
	strcpy(id,str);
      } else {
	return -1;
      }
    }
    data=CMGetKey(cop,"SystemName",NULL);
    if (data.type==CMPI_string && data.value.string) {
      str=CMGetCharPtr(data.value.string);
      if (strlen(str)<systemidlen) {
	strcpy(systemid,str);
	return 0;
      }
    }
  }
  return -1;
}

int GETRES (char *** resclasses)
{
  if (resclasses) {
    *resclasses = calloc(2,sizeof(char*));
    (*resclasses)[0] = strdup("Linux_IPProtocolEndpoint");
    return 0;
  }
  return -1;
}

void FREERES (char **resclasses)
{
  int i=0;
  if (resclasses) {
    while (resclasses[i]) {
      free(resclasses[i++]);
    }
    free(resclasses);
  }
}