#ifndef FET_H
#define FET_H

#include<stdint.h>


typedef uint32_t udword;
typedef int32_t  sdword;
typedef uint16_t uword;
typedef int16_t  sword;
typedef uint8_t  ubyte;
typedef int8_t   sbyte;
typedef uint16_t vaddr;
typedef uint32_t paddr;

struct device{
  void (*init)(void);
  void (*writeb)(paddr, ubyte);
  void (*writew)(paddr, uword);
  ubyte (*readb)(paddr);
  uword (*readw)(paddr);
  //void (*debug)(void (*)(const char *, const char *, ...));
  };

struct busmap{
  paddr start, end;
  struct busmap *next;
  struct device *device;
};



struct busmap *bus_maps;

void bus_mapdev(struct device *device, paddr start, paddr end);
ubyte bus_readb(paddr addr);
uword bus_readw(paddr addr);
void bus_writeb(paddr addr, ubyte val);
void bus_writew(paddr addr, uword val);

void cpu_writereg(int no, uword val);
uword cpu_readreg(int no);
void cpu_init();
void cpu_step();


#endif
