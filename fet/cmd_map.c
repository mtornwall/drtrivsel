#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include"fet.h"
#include"commands.h"


#define USAGE_ERROR() {usage(); return 1;}


void bus_devinit_usage(char *dev, char *usage){
  printf("Usage for device %s:  %s( %s )\n", dev, dev, usage);
  }

static void usage(){
  printf("Usage:  map <device_type(options)> [as <name>] at <base_addr>\n");
  }

int cmd_map(char **argv){
  device *devtype;
  device *dev;
  paddr addr;
  char *endptr;
  char *name=0, **devoptions=0;

  ++argv;

  if(!*argv){  // Display bus map
    for(dev=bus_mapped_devices; dev; dev=dev->next)
      printf("%06X - %06X  %10s\n", dev->start, dev->end, dev->devname);
    return 0;
    }
  if(!strcmp(*argv, "help")){
    usage();
    return 0;
    }
  devtype=dev_find_devtype(*argv);

  ++argv;
  if(!*argv)USAGE_ERROR()

  if(!strcmp(*argv, "(")){
    devoptions=argv+1;
    do{
      ++argv;
      if(!*argv)USAGE_ERROR()
      }while(strcmp(argv[-1], ")"));
    free(argv[-1]);                   // Null terminate device option list
    argv[-1]=0;                       // for device init
    }

  if(!strcmp(*argv, "as")){
    argv++;
    if(!*argv)USAGE_ERROR()
    name=*argv++;
    if(!*argv)USAGE_ERROR()
    }

  if(strcmp(*argv, "at"))USAGE_ERROR()

  ++argv;
  if(!*argv)USAGE_ERROR()

  addr=strtoul(*argv, &endptr, 0);
  if(*endptr)USAGE_ERROR()
  
  if(!devtype){
    printf("No such device type.\n");
    return 1;
    }

  ++argv;
  bus_mapdev(devtype, addr, name, devoptions);

  return 0;
  }
