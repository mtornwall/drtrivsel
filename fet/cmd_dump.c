#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include"fet.h"


static void usage(){
  printf("Usage:  dump <addr>\n");
  }

int cmd_dump(int again, char **argv){
  static paddr next=0;
  paddr potential;
  char *endptr;
  uword w;
  paddr temp;
  unsigned char c;
  int ok_count;

  if(!again && argv[1]){
    potential=strtoul(argv[1], &endptr, 0);
    if(*endptr)USAGE()
    next=potential;
    }

  for(int i=0; i<8; ++i){
    temp=next;
    printf("%06lX:", (unsigned long) next);
    ok_count=0;
    for(int j=0; j<8; j++){
      w=bus_readw(next);
      if(!bus_error_happened()){
        printf(" %04X", (unsigned) w);
        ++ok_count;
        }
      else
        printf("     ");
      next+=2;
      }
    next=temp;
    printf("  |");
    for(int j=0; j<ok_count; j++){
      w=bus_readw(next);
      next+=2;
      c=w>>8;
      putchar(isprint(c)?c:'.');
      c=w&255;
      putchar(isprint(c)?c:'.');
      }
    printf("|\n");
    if(catch_bus_error())return 1;
    }

  return 0;
  }
