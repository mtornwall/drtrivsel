#define _GNU_SOURCE
#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<assert.h>
#include<wordexp.h>
#include<string.h>
#include<readline/readline.h>
#include<readline/history.h>
#include"fet.h"
#include"commands.h"


void signal_bus_error(char *what, paddr where){
  fprintf(stderr, "*** Bus error: attempted %s @ %06X\n", what, where);
  }

device *find_dev(char *name){
  int i;
  for(i=0; i<ndevices; i++)
    if(!strcmp(devices[i]->name, name))return devices[i];
  return 0;
  }

int main(int argc, char *argv[]){
wordexp_t we;
char *line;
int i;
printf("%d devices: \n", ndevices);
for(i=0; i<ndevices; i++)printf("%s\n", devices[i]->name);
printf("%d commands: \n", ncommands);
for(i=0; i<ncommands; i++)printf("%s\n", commands[i].name);
/*
paddr base=0;
while((line=readline("> "))){
  if (line && *line)add_history(line);
  wordexp(line, &we, WRDE_SHOWERR);
  free(line);
  bus_mapdev(devices[0], base, 0, we.we_wordv);
  wordfree(&we);
  base+=0x40000;
  }
*/
  return 0;  
  }
