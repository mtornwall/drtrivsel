#define _GNU_SOURCE
#include<assert.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"fet.h"


void bus_mapdev(device *devtype, paddr start, char *name, char **argv){
  device **d, *dev;
  int occupied=0;

  if(!name)asprintf(&name, "%s%d", devtype->typename, devtype->n_mapped);
  else name=strdup(name);

  dev=devtype->create(name, start, argv);
  if(!dev){
    free(name);
    return;
    }

  dev->devname=name;

  d=&bus_mapped_devices;
  while(*d){
    if((*d)->start < dev->start){
      if((*d)->end > dev->start)
        occupied=1;
      }
    else
      if((*d)->start < dev->end)
        occupied=1;
    d=&((*d)->next);
    }

  if(occupied)
    printf("Address range not free.\n");

  if(occupied || dev->init(dev)){
    free(name);
    free(dev);
    return;
    }

  d=&bus_mapped_devices;
  while(*d && (*d)->start<start)d=&((*d)->next);

  dev->next = *d;
  *d = dev;

  ++(devtype->n_mapped);
  }

device *dev_find_mapped(char *name){
  device *dev=bus_mapped_devices;

  while(dev){
    if(!strcmp(dev->devname, name))return dev;
    dev=dev->next;
    }

  return 0;
  }

static device *addr_to_dev(paddr addr){
  device *dev=bus_mapped_devices;

  while(dev && dev->start <= addr){
    if(dev->end >= addr)return dev;
    dev=dev->next;
    }

  return 0;
  }

ubyte bus_readb(paddr addr){
  device *dev=addr_to_dev(addr);
  if(!dev)signal_bus_error("byte read", addr);
  return dev->readb(dev, addr-dev->start);
  }

uword bus_readw(paddr addr){
  addr &= ~1;
  device *dev=addr_to_dev(addr);
  if(!dev)signal_bus_error("word read", addr);
  return dev->readw(dev, addr-dev->start);
  }

void bus_writeb(paddr addr, ubyte val){
  device *dev=addr_to_dev(addr);
  if(!dev)signal_bus_error("byte write", addr);
  dev->writeb(dev, addr-dev->start, val);
  }

void bus_writew(paddr addr, uword val){
  addr &= ~1;
  device *dev=addr_to_dev(addr);
  if(!dev)signal_bus_error("word write", addr);
  dev->writew(dev, addr-dev->start, val);
  }
