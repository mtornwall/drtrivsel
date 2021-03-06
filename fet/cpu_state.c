#include<assert.h>
#include<stdlib.h>
#include<string.h>
#include"fet.h"


static uword r[16];
static vaddr pc;
static cpu_flags flags;

#define C_FLAG 1
#define Z_FLAG 2
#define N_FLAG 4
#define V_FLAG 8


void cpu_write_reg(int no, uword val){
  assert(no >= 0 && no < 16);
  if(no)r[no] = val;
  }

uword cpu_read_reg(int no){
  assert(no >= 0 && no < 16);
  return r[no];
  }

void cpu_state_init(){
  }

void cpu_write_pc(vaddr new_pc){
  pc=new_pc;
  }

vaddr cpu_read_pc(){
  return pc;
  }

vaddr cpu_read_inc_pc(){
  vaddr _pc=pc;
  pc+=2;
  return _pc;
  }

cpu_flags cpu_read_flags(){
  return flags;
  }

void cpu_write_cflag(int f){
  flags.c=!!f;
  }

void cpu_write_zflag(int f){
  flags.z=!!f;
  }

void cpu_write_nflag(int f){
  flags.n=!!f;
  }

void cpu_write_vflag(int f){
  flags.n=!!f;
  }

uword cpu_read_flag_uword(){
  uword f=0;

  f|=flags.c?C_FLAG:0;
  f|=flags.z?Z_FLAG:0;
  f|=flags.n?N_FLAG:0;
  f|=flags.v?V_FLAG:0;

  return f;
  }

void cpu_write_flag_uword(uword f){
  flags.c=!!(f&C_FLAG);
  flags.z=!!(f&Z_FLAG);
  flags.n=!!(f&N_FLAG);
  flags.v=!!(f&V_FLAG);
  }
