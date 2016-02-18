#define _GNU_SOURCE
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include"fet.h"


typedef struct{
  device dev;
  paddr base;
  uword mem[];
  }mem_block;

device dev_mem;  // Defined at the bottom of this file


static void usage(){
  fprintf(stderr, "Usage:  mem <length>\n\n");
  }

static struct device *init(char *name, paddr start, char **argv){
  char *endptr;
  paddr len;
  mem_block *mb;

  if(!argv[0] || !argv[0][0]){
    fprintf(stderr, "Missing length parameter.\n");
    usage();
    return 0;
    }

  len=strtoul(argv[0], &endptr, 0);  // Bytes
  len=(len+1)/2;                     // Words

  if(*endptr){
    fprintf(stderr, "Malformed length parameter.\n");
    usage();
    return 0;
    }

  mb=malloc(sizeof(*mb)+len*sizeof(mb->mem[0]));
  mb->dev=dev_mem;
  mb->dev.name=strdup(name);
  mb->dev.map_length=len;
  mb->base=start;

  return (device *)mb;
  }

static void writeb(device *_dev, paddr _addr, ubyte d){
  mem_block *dev=(mem_block *)_dev;
  paddr addr=_addr-dev->base;

  if(addr&1){
    dev->mem[addr/2]&=0xFF00;
    dev->mem[addr/2]|=d;
    }
  else{
    dev->mem[addr/2]&=0x00FF;
    dev->mem[addr/2]|=((uword)d)<<8;
    }
  }

static void writew(device *_dev, paddr _addr, uword d){
  mem_block *dev=(mem_block *)_dev;
  paddr addr=_addr-dev->base;

  dev->mem[addr/2]=d;
  }

static ubyte readb(device *_dev, paddr _addr){
  mem_block *dev=(mem_block *)_dev;
  paddr addr=_addr-dev->base;

  if(addr&1)
    return dev->mem[addr/2]&0xFF;
  else
    return dev->mem[addr/2]>>8;
  }

static uword readw(device *_dev, paddr _addr){
  mem_block *dev=(mem_block *)_dev;
  paddr addr=_addr-dev->base;

  return dev->mem[addr/2];
  }

static void console_command(device *_dev, char **firstline){
  mem_block *dev=(mem_block *)_dev;

  }

static int lookup_reg(paddr *addr, device *_dev, char *name){
  mem_block *dev=(mem_block *)_dev;

  return 0;
  }


device dev_mem={
  "mem", 0, 0,
  init, writeb, writew, readb, readw, console_command, lookup_reg
  };
