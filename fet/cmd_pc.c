#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include"fet.h"


static void usage(){
  printf("Usage:  pc [= <addr>]\n");
  }

int cmd_pc(int again, char **argv){
  uword addr;
  char *endptr;

  ++argv;

  if(!*argv)return runcmd("dis -n %d", cpu_read_pc());

  if(strcmp(*argv, "="))USAGE()

  ++argv;
  addr=strtoul(*argv, &endptr, 0);
  if(*endptr)USAGE()

  cpu_write_pc(addr);
  return runcmd("dis -n %d", addr);
  }
