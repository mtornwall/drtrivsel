#define _GNU_SOURCE

#include<stdarg.h>
#include<stdlib.h>
#include<stdio.h>
#include"fet.h"


int runcmd(char *fmt, ...){
  va_list ap;
  char *cmdstr, **toklist;
  int toklist_len;
  command *cmd;
  int ret;

  va_start(ap, fmt);
  if(vasprintf(&cmdstr, fmt, ap)<0)return -1;
  va_end(ap);

  toklist_len=cmdlex(&toklist, cmdstr);
  free(cmdstr);

  cmd=find_cmd(toklist[0]);
  ret=cmd->function(0, toklist);
  free_toklist(toklist, toklist_len);

  return ret;
  }
