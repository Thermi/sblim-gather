/*
 * $Id: dirutil.c,v 1.1 2006/02/08 20:26:46 mihajlov Exp $
 *
 * (C) Copyright IBM Corp. 2003
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
 * Description: Directory Utilities
 */

#include "dirutil.h"
#include <dirent.h>
#include <stddef.h>
#include <string.h>
#include <malloc.h>
#include <fnmatch.h>

int  getfilelist(const char *dir, const char *pattern, char ***names)
{
  struct dirent **entries;
  int n,i,j; 
  
  if (dir==NULL||names==NULL) {
    return -1;
  }
  
  n = scandir(dir, &entries, 0, alphasort);
  if (n > 0) {
    *names = calloc(n,sizeof(char*));
    i=j=0;
    while(i<n) {
      if (pattern == NULL || fnmatch(pattern,entries[i]->d_name,0) == 0) {
	(*names)[j] = strdup(entries[i]->d_name);
	j += 1;
      }
      i += 1;
    } 
    (*names)[j] = NULL;
    return 0;
  }
  return -1;
}

void releasefilelist(char **names)
{
  int i;
  if (names) {
    for (i=0; names[i]; i++) {
      free(names[i]);
    }
    free(names);
  }
}
