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

typedef struct cpu_flags{
  unsigned c:1, z:1, n:1, v:1;
  } cpu_flags;


struct busmap *bus_maps;

void bus_mapdev(struct device *device, paddr start, paddr end);
ubyte bus_readb(paddr addr);
uword bus_readw(paddr addr);
void bus_writeb(paddr addr, ubyte val);
void bus_writew(paddr addr, uword val);

void cpu_write_reg(int no, uword val);
uword cpu_read_reg(int no);
vaddr cpu_read_pc();
void cpu_write_pc(vaddr new_pc);
vaddr cpu_read_inc_pc();
cpu_flags cpu_read_flags();
void cpu_write_cflag(int f);
void cpu_write_zflag(int f);
void cpu_write_nflag(int f);
void cpu_write_vflag(int f);
uword cpu_read_flag_uword();
void cpu_write_flag_uword(uword f);

void cpu_init();
void cpu_step();


#endif
