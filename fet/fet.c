#define _GNU_SOURCE
#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include<readline/readline.h>
#include<readline/history.h>
#include<signal.h>
#include"fet.h"
#include"commands.h"

static volatile int running;
static int show_bus_errors=1;

#define BERROFF() show_bus_errors=0;
#define BERRON() show_bus_errors=1;

void sigint(int foo){
  running=0;
  }

void signal_bus_error(char *what, paddr where){
  if(show_bus_errors)
    fprintf(stderr, "*** Bus error: attempted %s @ %06X\n", what, where);
  running=0;
  }

int cmd_step(int again, char **argv){
  static char dis[30];
  int len;

  cpu_step();

  BERROFF()
  len=disassemble(dis, cpu_read_pc());
  printf("next: %04X: %04X", cpu_read_pc(), bus_readw(cpu_read_pc()));
  if(len==2)printf(" %04X", bus_readw(cpu_read_pc()+2));
  else printf("     ");
  printf("   %s\n", dis);
  BERRON()

  return 0;
  }

int cmd_run(int again, char **argv){
  running=1;
  while(running)cpu_step();
  printf("Stopped.\n");
  return 0;
  }

int cmd_x(int again, char **argv){
  // ***
  return 1;
  }

int cmd_d(int again, char **argv){
  // ***
  return 1;
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
