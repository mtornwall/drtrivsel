#ifndef COMMANDS_H
#define COMMANDS_H

typedef struct{
  char *name;
  int (*function)(int, char **);
  }command;

extern command commands[];
int ncommands;

#endif
