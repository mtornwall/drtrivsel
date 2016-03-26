#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<stdarg.h>
#include"fet.h"


void devinit_usage(char *dev, ...){
  char *usage;
  va_list ap;

  printf("Usage for device %s:", dev);

  va_start(ap, dev);
  if(va_arg(ap, char *) && va_arg(ap, char *))putchar('\n');
  va_end(ap);

  va_start(ap, dev);

  while((usage=va_arg(ap, char *)))
    printf("  %s( %s )\n", dev, usage);

  va_end(ap);
  }

static void usage(){
  printf("Usage:  map <device_type(options)> [as <name>] at <base_addr>\n");
  }

int cmd_map(int again, char **argv){
  device *devtype;
  device *dev;
  paddr addr=0xFFFFFFFFL;
  char *endptr;
  char *name=0, **devoptions=0;
  int parcount;

  ++argv;

  if(!*argv){  // Display bus map
    for(dev=bus_mapped_devices; dev; dev=dev->next)
      printf("%06lX - %06lX  %10s\n",
             (unsigned long)dev->start, (unsigned long)dev->end, dev->devname);
    return 0;
    }
  if(!strcmp(*argv, "help")){
    usage();
    return 0;
    }
  devtype=dev_find_devtype(*argv);
  ++argv;

  if(!*argv)USAGE()
  if(!strcmp(*argv, "(")){
    devoptions=argv+1;
    parcount=1;
    do{
      ++argv;
      if(!*argv)USAGE()
      if(!strcmp(*argv, ")"))
        --parcount;
      else if(!strcmp(*argv, "("))
        ++parcount;
      }while(parcount);
    free(*argv);                     // Null terminate device option list
    *argv=0;                         // for device init
    }
  ++argv;

  while(*argv){
    if(!strcmp(*argv, "as")){
      ++argv;
      if(!*argv)USAGE()
      name=*argv++;
      }
    else if(!strcmp(*argv, "at")){
      ++argv;
      if(!*argv)USAGE()
      addr=strtoul(*argv, &endptr, 0);
      if(*endptr)USAGE()
      ++argv;
      }
    else USAGE()
    }

  if(addr==0xFFFFFFFFL)USAGE()

  if(!devtype){
    printf("No such device type.\n");
    return 1;
    }

  return bus_mapdev(devtype, addr, name, devoptions);
  }
