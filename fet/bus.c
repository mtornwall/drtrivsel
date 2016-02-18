#define _GNU_SOURCE
#include<assert.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"fet.h"


void bus_mapdev(device *dev, paddr start, char *name, char **argv){
  busmap **m, *map = malloc(sizeof *map);

  if(!name)asprintf(&(map->name), "%s%d", dev->name, dev->n_mapped);
  else map->name=strdup(name);

  map->dev=dev->init(map->name, start, argv);

  map->start = start;
  map->end = start+map->dev->map_length;

  m=&bus_maps;
  while(*m && (*m)->start<start)m=&((*m)->next);

  map->next = *m;
  *m = map;

  ++(dev->n_mapped);
  }

static busmap *find_map(paddr addr){
  busmap *map=bus_maps;

  while(map && map->start <= addr){
    if(map->end >= addr)return map;
    map=map->next;
    }

  return 0;
  }

ubyte bus_readb(paddr addr){
  busmap *map=find_map(addr);
  if(!map)signal_bus_error("byte read", addr);
  device *dev=map->dev;
  return dev->readb(dev, addr-map->start);
  }

uword bus_readw(paddr addr){
  addr &= ~1;
  busmap *map=find_map(addr);
  if(!map)signal_bus_error("word read", addr);
  device *dev=map->dev;
  return dev->readw(dev, addr-map->start);
  }

void bus_writeb(paddr addr, ubyte val){
  busmap *map=find_map(addr);
  if(!map)signal_bus_error("byte write", addr);
  device *dev=map->dev;
  dev->writeb(dev, addr-map->start, val);
  }

void bus_writew(paddr addr, uword val){
  addr &= ~1;
  busmap *map=find_map(addr);
  if(!map)signal_bus_error("word write", addr);
  device *dev=map->dev;
  dev->writew(dev, addr-map->start, val);
  }
