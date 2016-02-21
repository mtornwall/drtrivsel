#include<stdio.h>
#include"fet.h"
#include"commands.h"


int cmd_help(char **argv){
  int i;

  printf("help help help\n");

  for(i=1; argv[i]; i++)
    printf("'%s', ", argv[i]);
  putchar('\n');

  return 0;
  }
