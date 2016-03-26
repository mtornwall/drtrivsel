#define _GNU_SOURCE
#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include<readline/readline.h>
#include<readline/history.h>
#include<signal.h>
#include"fet.h"

static volatile int running;
static int bus_error;
static paddr bus_error_addr;

void sigint(int foo){
  running=0;
  }

int catch_bus_error(){
  if(!bus_error)return 0;
  printf("*** Bus error: attempted %s %s @ %06lX\n",
         (bus_error&BERR_WORD)?"word":"byte",
         (bus_error&BERR_READ)?"read":"write",
         (long)bus_error_addr);
  bus_error=0;
  return 1;
  }

int bus_error_happened(){
  return bus_error;
  }

void signal_bus_error(int what, paddr where){
  if(!bus_error){
    bus_error=what;
    bus_error_addr=where;
    running=0;
    }
  }

int cmd_step(int again, char **argv){
  cpu_step();
  if(catch_bus_error())return 1;

  return runcmd("dis -n %d", cpu_read_pc());
  }

int cmd_run(int again, char **argv){
  int ret=0;

  running=1;
  while(running)cpu_step();
  ret=catch_bus_error();
  printf("Stopped.\n");

  return ret;
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
  int toklist_len;
  command *cmd;
  char **toklist;
  struct sigaction sa;
  HIST_ENTRY *last;
  int again;

  sa.sa_handler=sigint;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags=0;
  sigaction(SIGINT, &sa, 0);

  cpu_init();

  while((line=readline("fet> "))){
    again=0;
    if(line){

      if(!*line){
        last=history_get(history_length);
        if(last){
          free(line);
          line=strdup(last->line);
          again=1;
          }
        }

      toklist_len=cmdlex(&toklist, line);

      if(toklist_len && !again)
        add_history(line);
      free(line);

      if(toklist_len<0)
        printf("Syntax error.\n");
      if(toklist_len<1)
        continue;

      if((cmd=find_cmd(toklist[0])))
        cmd->function(again, toklist);
      else if(dev_find_mapped(toklist[0]))
        printf("mapped device\n");
      else if(dev_find_devtype(toklist[0]))
        printf("device type\n");
      else printf("NO\n");

      free_toklist(toklist, toklist_len);
      }

    }

  return 0;  
  }
