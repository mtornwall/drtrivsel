#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include"fet.h"


static void usage(){
  printf("Usage:  deposit -bw|<addr>[,|=]<val>[,] ...\n");
  }

int cmd_deposit(int again, char **argv){
  unsigned long n;
  paddr addr;
  char *endptr;
  int bytemode=0, seen_addr=0, addr_allowed=1;

  ++argv;

  if(!*argv)USAGE()

  while(*argv){
    if(**argv=='-'){
      if(!strcmp(*argv, "-b"))bytemode=1;
      else if(!strcmp(*argv, "-w"))bytemode=0;
      else{printf("Unrecognised option \"%s\"\n", *argv); return 1;}
      argv++;
      }
    else{
      n=strtoul(*argv, &endptr, 0);
      if(*endptr)USAGE()
      ++argv;
      if(addr_allowed && (!seen_addr || (*argv && !strcmp(*argv, "=")))){
        if(n>0xFFFFFF){
          printf("Number out of range for address: %s\n", *(argv-1));
          return 1;
          }
        addr=n;
        seen_addr=1;
        addr_allowed=0;
        }
      else{
        if(!addr_allowed){
          if(*argv && !strcmp(*argv, "=")){
            printf("Data missing before address %s\n", *(argv-1));
            return 1;
            }
          addr_allowed=1;
          }
        if(bytemode){
          if(n>0xFF){
            printf("Number out of range for byte data: %s\n", *(argv-1));
            return 1;
            }
          bus_writeb(addr, n);
          if(catch_bus_error())return 1;
          }
        else{
          if(n>0xFFFF){
            printf("Number out of range for word data: %s\n", *(argv-1));
            return 1;
            }
          bus_writew(addr, n);
          if(catch_bus_error())return 1;
          }
        addr+=bytemode?1:2;
        }
      if(*argv && (!strcmp(*argv, ",") || !strcmp(*argv, "=")))++argv;
      }
    }

  return 0;
  }
