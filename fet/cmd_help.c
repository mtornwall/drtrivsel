#define _GNU_SOURCE

#include<stdio.h>
#include"fet.h"


int cmd_help(int again, char **argv){
  int (*f)(int, char **)=0;

  printf("Commands:\n");

  FILE *c=columns();
  for(int i=0; i<ncommands; i++){
    fprintf(c, (f==commands[i].function)?"/%s":"\n  %s", commands[i].name);
    f=commands[i].function;
    }
  fputc('\n', c);
  pclose(c);

  return 0;
  }
