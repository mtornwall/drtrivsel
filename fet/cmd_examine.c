#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include"fet.h"


static void usage(){
  printf("Usage:  examine -bw|<addr> ...\n");
  }

int cmd_examine(int again, char **argv){
  paddr addr=0;
  char *endptr;
  int bytemode=0;

  ++argv;

  if(!*argv)USAGE()

  while(*argv){
    if(**argv=='-'){
      if(!strcmp(*argv, "-b"))bytemode=1;
      else if(!strcmp(*argv, "-w"))bytemode=0;
      else{printf("Unrecognised option \"%s\"\n", *argv); return 1;}
      }
    else if(strcmp(*argv, ",")){
      addr=strtoul(*argv, &endptr, 0);
      if(*endptr)USAGE()
      printf("%06lX: ", (unsigned long) addr);
      if(!bytemode)printf("%06X\n", (unsigned) bus_readw(addr));
      else printf("%02X\n", (unsigned) bus_readb(addr));
      if(catch_bus_error())return 1;
      }
    ++argv;
    }

  return 0;
  }
