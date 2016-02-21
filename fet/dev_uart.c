#include<stdlib.h>
#include<stdio.h>
#include"fet.h"


typedef struct{
  device dev;
  }uart;

device dev_uart;  // Defined at the bottom of this file


static device *create(char *name, paddr start, char **argv){
  uart *uart;
printf("uart create\n");
for(; *argv; argv++)printf(" %s", *argv);
putchar('\n');

  uart=malloc(sizeof(*uart));
  uart->dev=dev_uart;
  uart->dev.argv_temp=argv;
  uart->dev.start=start;
  uart->dev.end=start+3;
  uart->dev.n_mapped=-1;     // So we can tell that this device struct is a
                             // mapped device and not the devtype struct
  return (device *)uart;
  }

static int init(device *dev){
printf("uart init");
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
  "uart",            // typename
  0, 0, 0, 0, 0, 0,  // devname, n_mapped, argv_temp, start, end, next
  create, init, writeb, writew, readb, readw, console_command, lookup_reg
  };
