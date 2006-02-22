
/*
 * $Id: cimplugXen.c,v 1.1 2006/02/22 14:11:59 mihajlov Exp $
 *
 * (C) Copyright IBM Corp. 2006
 *
 * THIS FILE IS PROVIDED UNDER THE TERMS OF THE COMMON PUBLIC LICENSE
 * ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
 * CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
 *
 * You can obtain a current copy of the Common Public License from
 * http://oss.software.ibm.com/developerworks/opensource/license-cpl.html
 *
 * Author:       Oliver Benke (benke@de.ibm.com)
 * Contributors: 
 *
 * Description:
 * CIM Id Plugin for Xen specific metrics
 *
 * assumption is that all metrics will be associated to Xen_ComputerSystem for now, 
 * representing Xen domain virtual machines
 */

#include <cimplug.h>
#include <string.h>
#include <stdlib.h>


CMPIObjectPath* COP4VALID (CMPIBroker *broker, const char *id, 
			const char *systemid)
{
  /* we construct the operating system id according to the OSBase
     rules */
  CMPIObjectPath *cop = CMNewObjectPath(broker,NULL,"Xen_ComputerSystem",
					NULL);
  if (cop) {
    CMAddKey(cop,"Name",systemid,CMPI_chars);
    CMAddKey(cop,"CreationClassName","Xen_ComputerSystem",CMPI_chars);
    CMAddKey(cop,"CSName",systemid,CMPI_chars);
    CMAddKey(cop,"CSCreationClassName","Linux_ComputerSystem",
	     CMPI_chars);
  }
  return cop;
}

int VALID4COP (CMPIObjectPath *cop, char *id, size_t idlen,
	    char *systemid, size_t systemidlen)
{
  CMPIData data;
  char    *str;
  
  if (cop && id && systemid) {
    if (strlen("OperatingSystem")+1 >idlen) {
      return -1;
    } else {
      strcpy(id,"OperatingSystem");
    }
    data=CMGetKey(cop,"CSName",NULL);
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
    (*resclasses)[0] = strdup("Xen_ComputerSystem");
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


