#include<stdio.h>
#include"fet.h"
#include"commands.h"


static void usage(){
  printf("Usage:  map <device_type> <base_addr> name pars");
  }

int cmd_map(char **argv){
  device *dev;

  if(!(argv[1] && (dev=find_dev(argv[1]))));
  }
