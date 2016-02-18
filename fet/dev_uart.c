#include<stdio.h>
#include"fet.h"


static struct device *init(char *name, paddr start, char **argv){
printf("uart init");
for(; *argv; argv++)printf(" %s", *argv);
putchar('\n');
return 0;
  }

static void writeb(device *dev, paddr a, ubyte d){
  }

static void writew(device *dev, paddr a, uword d){
  }

static ubyte readb(device *dev, paddr a){
  return 0;
  }

static uword readw(device *dev, paddr a){
  return 0;
  }

static void console_command(device *dev, char **firstline){
  }

static int lookup_reg(paddr *addr, device *dev, char *name){
  return 0;
  }


device dev_uart={
  "uart", 0, 3,
  init, writeb, writew, readb, readw, console_command, lookup_reg
  };
