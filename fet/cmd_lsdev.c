#define _GNU_SOURCE

#include<stdio.h>
#include"fet.h"
#include"commands.h"


int cmd_lsdev(int again, char **argv){
  FILE *c=columns();
  for(int i=0; i<ndevtypes; i++)
    fprintf(c, "%s\n", devtypes[i]->typename);
  pclose(c);

  return 0;
  }
