#include "mcfg.h"
#include <stdio.h>

int main()
{
  const char * allowed[] = {
    "great",
    "medium",
    "small",
    NULL
  };
  char cfgbuf[10];

  if (set_configfile("./test.cfg",allowed)) {
    printf("Failed to open config file %s\n","./tesst.cfg");
  }
  if (get_configitem("great",cfgbuf,sizeof(cfgbuf))==0) {
    printf("Config value for %s is %s\n","great",cfgbuf);
  } else {
    printf("Failed to get config value for %s\n","great");
  }
  if (get_configitem("medium",cfgbuf,sizeof(cfgbuf))==0) {
    printf("Config value for %s is %s\n","medium",cfgbuf);
  } else {
    printf("Failed to get config value for %s\n","medium");
  }
  if (get_configitem("small",cfgbuf,sizeof(cfgbuf))==0) {
    printf("Config value for %s is %s\n","small",cfgbuf);
  } else {
    printf("Failed to get config value for %s\n","small");
  }
  if (get_configitem("huge",cfgbuf,sizeof(cfgbuf))==0) {
    printf("Config value for %s is %s\n","huge",cfgbuf);
  } else {
    printf("Failed to get config value for %s\n","huge");
  }

  return 0;
}
