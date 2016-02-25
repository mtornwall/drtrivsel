#define _GNU_SOURCE
#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include<readline/readline.h>
#include<readline/history.h>
#include"fet.h"
#include"commands.h"

static volatile int running;
static int show_bus_errors=1;

#define BERROFF() show_bus_errors=0;
#define BERRON() show_bus_errors=1;

void signal_bus_error(char *what, paddr where){
  if(show_bus_errors)
    fprintf(stderr, "*** Bus error: attempted %s @ %06X\n", what, where);
  running=0;
  }

int cmd_step(char **argv){
  cpu_step();

  BERROFF()
  printf("next: %04X: %04X\n", cpu_read_pc(), bus_readw(cpu_read_pc()));
  BERRON()

  return 0;
  }

int cmd_run(char **argv){
  running=1;
  while(running)cpu_step();
  printf("Stopped.\n");

  return 0;
  }

device *dev_find_devtype(char *name){
  int i;
  for(i=0; i<ndevtypes; i++)
    if(!strcmp(devtypes[i]->typename, name))return devtypes[i];
  return 0;
  }

command *find_cmd(char *name){
  int i;

  for(i=0; i<ncommands; i++){
    if(!strcmp(name, commands[i].name))return commands+i;
    }

  return 0;
  }

int main(int argc, char *argv[]){
  char *line;
  int i, toklist_len;
  command *cmd;
  char **toklist;

  cpu_init();

  while((line=readline("fet> "))){
    if(line && *line){
      toklist_len=cmdlex(&toklist, line);

      if(toklist_len)
        add_history(line);
      free(line);

      if(toklist_len<0)
        printf("Syntax error.\n");
      if(toklist_len<1)
        continue;

//for(i=0; toklist[i]; i++)printf("'%s'  ", toklist[i]);
//putchar('\n');

      if((cmd=find_cmd(toklist[0]))){
        printf("command\n");
        cmd->function(toklist);
        }

      else if(dev_find_mapped(toklist[0])){
        printf("mapped device\n");
        }

      else if(dev_find_devtype(toklist[0])){
        printf("device type\n");
        }

      else{
        printf("NO\n");
        }

      free_toklist(toklist, toklist_len);
      }
    }

  return 0;  
  }
