#define _GNU_SOURCE

#include<stdio.h>
#include"fet.h"
#include"commands.h"


int cmd_help(int again, char **argv){
  printf("Commands:\n");
  FILE *c=columns();
  for(int i=0; i<ncommands; i++)
    fprintf(c, "  %s\n", commands[i].name);
  pclose(c);

  return 0;
  }
