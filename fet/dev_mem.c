#define _GNU_SOURCE
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include"fet.h"

#include<time.h>


typedef struct{
  device dev;
  uword *mem;
  }mem_block;

device dev_mem;  // Defined at the bottom of this file

static void usage(){
  devinit_usage("mem", "<size>", 0);
  }

static device *create(char *name, paddr start, char **argv){
  mem_block *mb;
  char *endptr;
  paddr len;

  if(!argv || !*argv || !**argv)
    USAGE_0()

  len=strtoul(*argv, &endptr, 0);  // Bytes

  if(*endptr || !len){
    if(!strcmp(endptr, "k") || !strcmp(endptr, "K"))len*=1024;
    else if(!strcmp(endptr, "M"))len*=1024*1024;
    else USAGE_0()
    }

  ++argv;

  len=(len+1)/2;                     // Words

  mb=malloc(sizeof(*mb));
  mb->dev=dev_mem;
  mb->dev.argv_temp=argv;
  mb->dev.start=start;
  mb->dev.end=start+len*2-1;
  mb->dev.n_mapped=-1;       // So we can tell that this device struct is a
                             // mapped device and not the devtype struct
  return (device *)mb;
  }

static int init(device *dev){
  mem_block *mb=(mem_block *)dev;

  if(*(mb->dev.argv_temp)){
    usage();
    return -1;
    }

  mb->mem=malloc((mb->dev.end - mb->dev.start)*sizeof(mb->mem[0]));

srand(time(0));
for(unsigned i=0; i<(mb->dev.end - mb->dev.start); i++){
  unsigned r;
  r=rand();
  mb->mem[i]=((i&3)<<14)|(((i>>2)&15)<<10)|(r&1023);
  }

  return 0;
  }

static void writeb(device *_dev, paddr _addr, ubyte d){
  mem_block *dev=(mem_block *)_dev;
  paddr addr=_addr-_dev->start;

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
  paddr addr=_addr-_dev->start;

  dev->mem[addr/2]=d;
  }

static ubyte readb(device *_dev, paddr _addr){
  mem_block *dev=(mem_block *)_dev;
  paddr addr=_addr-_dev->start;

  if(addr&1)
    return dev->mem[addr/2]&0xFF;
  else
    return dev->mem[addr/2]>>8;
  }

static uword readw(device *_dev, paddr addr){
  mem_block *dev=(mem_block *)_dev;

  return dev->mem[addr/2];
  }

static void console_command(device *_dev, char **firstline){
//  mem_block *dev=(mem_block *)_dev;

  }

static int lookup_reg(paddr *addr, device *_dev, char *name){
// mem_block *dev=(mem_block *)_dev;

  return 0;
  }


device dev_mem={
  "mem",             // typename
  0, 0, 0, 0, 0, 0,  // devname, n_mapped, argv_temp, start, end, next
  create, init, writeb, writew, readb, readw, console_command, lookup_reg
  };

uword *mem_get_block(device *_dev){
  mem_block *dev=(mem_block *)_dev;

  return dev->mem;
  }
