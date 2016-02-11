#include<assert.h>
#include<stdlib.h>
#include"fet.h"


static struct device *addrspace[65536];

void bus_mapdev(struct device *device, paddr start, paddr end){
  struct busmap *map = malloc(sizeof *map);
  map->start = start;
  map->end = end;
  map->device = device;
  map->next = bus_maps;
  bus_maps = map;

  for (; start < end; ++start){
    assert(!addrspace[start]);
    addrspace[start] = device;
    }
  }

ubyte bus_readb(paddr addr){
  assert(addrspace[addr]);
  return addrspace[addr]->readb(addr);
  }

uword bus_readw(paddr addr){
  addr &= ~1;
  assert(addrspace[addr]);
  return addrspace[addr]->readw(addr);
  }

void bus_writeb(paddr addr, ubyte val){
  assert(addrspace[addr]);
  addrspace[addr]->writeb(addr, val);
  }

void bus_writew(paddr addr, uword val){
  addr &= ~1;
  addrspace[addr]->writew(addr, val);
  }
