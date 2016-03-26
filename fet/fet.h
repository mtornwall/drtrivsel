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
device *addr_to_dev(paddr);

device *bus_mapped_devices;

#define BERR_READ   1
#define BERR_WRITE  2
#define BERR_WORD   4
#define BERR_BYTE   8
#define BERR_RW (BERR_READ|BERR_WORD)
#define BERR_RB (BERR_READ|BERR_BYTE)
#define BERR_WW (BERR_WRITE|BERR_WORD)
#define BERR_WB (BERR_WRITE|BERR_BYTE)

void signal_bus_error(int what, paddr where);
int bus_error_happened();
int catch_bus_error();

uword *mem_get_block(device *);


typedef struct cpu_flags{
  unsigned c:1, z:1, n:1, v:1;
  } cpu_flags;


void devinit_usage(char *dev, ...);

int bus_mapdev(device *device, paddr start, char *name, char **argv);
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
int next_hex_dis(vaddr next);

void cpu_init();
void cpu_step();

int cmdlex(char ***to, char *str);
void free_toklist(char **toklist, int len);

extern char *regname[16];
int hex_dis(vaddr next);

FILE *columns(void);


#define USAGE() {usage(); return 1;}
#define USAGE_0() {usage(); return 0;}
#define DEFAULT_USAGE() if(!argv[1])USAGE()


typedef struct{
  char *name;
  int (*function)(int, char **);
  }command;

extern command commands[];
int ncommands;

command *find_cmd(char *name);

int runcmd(char *fmt, ...);

#endif
