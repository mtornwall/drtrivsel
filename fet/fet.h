#ifndef FET_H
#define FET_H

#include<stdint.h>
#include<stdio.h>


typedef uint32_t udword;
typedef int32_t  sdword;
typedef uint16_t uword;
typedef int16_t  sword;
typedef uint8_t  ubyte;
typedef int8_t   sbyte;
typedef uint16_t vaddr;
typedef uint32_t paddr;

typedef struct device device;
struct device{
  char *typename, *devname;
  int n_mapped;
  char **argv_temp;
  paddr start, end;
  device *next;
  device *(*create)(char *name, paddr start, char **argv);
  int (*init)(device *dev);
  void (*writeb)(device *dev, paddr, ubyte);
  void (*writew)(device *dev, paddr, uword);
  ubyte (*readb)(device *dev, paddr);
  uword (*readw)(device *dev, paddr);
  void (*console_command)(device *dev, char **firstline);
  int (*lookup_reg)(paddr *addr, device *dev, char *name);
  };

extern device *devtypes[];
int ndevtypes;

device *dev_find_devtype(char *name);
device *dev_find_mapped(char *name);

device *bus_mapped_devices;

void signal_bus_error(char *what, paddr where);

typedef struct cpu_flags{
  unsigned c:1, z:1, n:1, v:1;
  } cpu_flags;


void devinit_usage(char *dev, ...);

void bus_mapdev(device *device, paddr start, char *name, char **argv);
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

int cmdlex(char ***to, char *str);
void free_toklist(char **toklist, int len);

extern char *regname[16];
int disassemble(char *to, paddr addr);

FILE *columns(void);

#endif
