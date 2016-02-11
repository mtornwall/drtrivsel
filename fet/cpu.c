#include<assert.h>
#include"fet.h"


#define L_FIELD(instr) ((instr)&0x0020)
#define I_FIELD(instr) ((instr)&0x0010)
#define R_FIELD(instr) ((instr)&0x000F)
#define A_FIELD(instr) (((instr)&0x03C0)>>6)

static uword r[16];
static vaddr pc;
static uword flags;

#define C_FLAG 1
#define Z_FLAG 2
#define N_FLAG 4
#define V_FLAG 8

void cpu_writereg(int no, uword val){
  assert(no >= 0 && no < 16);
  if(no)r[no] = val;
  }

uword cpu_readreg(int no){
  assert(no >= 0 && no < 16);
  return r[no];
  }

void cpu_init(){
  }

void cpu_setpc(vaddr new_pc){
  pc=new_pc;
  }

uword readw_yop(uword instr){
  uword operand=cpu_readreg(R_FIELD(instr));
  if(L_FIELD(instr))operand+=bus_readw(pc++);
  if(I_FIELD(instr))operand=bus_readw(operand);
  return operand;
  }

void writew_yop(uword instr, uword val){
// ***
  }

void reflect_in_flags_zn(uword val){
  flags=(flags & ~(Z_FLAG | N_FLAG));
  if(!val)flags|=Z_FLAG;
  if(val&0x8000)flags|=N_FLAG;
  }

/*
void reflect_in_flags_cv(ulong val){

  }
*/

void cpu_step(){
  uword instr=bus_readw(pc++);

  if(!(instr&0x8000)){          // ALU-op           0 ooooo aaaa l i rrrr
    uword b=readw_yop(instr);
    uword a=cpu_readreg(A_FIELD(instr));
    uword result;
    if(!(instr&0x4000)){        // Arithmetic       0 0sc.f aaaa l i rrrr
      uword carry_in=(instr&0x2000)?!!(flags&C_FLAG):0;
      if(instr&0x1000)carry_in=1-carry_in;

      if(instr&0x0400)
        result=a+b+carry_in;    // add, adc, inc, b+0, b+c, shl, rol, $1
      else
        result=a-b-1+carry_in;  // sub, sbb, dec, b-0, b-1+c, -c
      // *** fixa carry out!
      }
    else{                       // Logic            0 1oooo aaaa l i rrrr
      switch(instr&0x3C){
        case  0: result = ~a      ; break;
        case  1: result = ~a | ~b ; break;
        case  2: result = ~a &  b ; break;
        case  3: result =  0      ; break;
        case  4: result = ~(a & b); break;
        case  5: result =      ~b ; break;
        case  6: result =  a ^  b ; break;
        case  7: result =  a & ~b ; break;
        case  8: result = ~a |  b ; break;
        case  9: result = ~a ^ ~b ; break;
        case 10: result =       b ; break;
        case 11: result =  a &  b ; break;
        case 12: result = ~0      ; break;
        case 13: result =  a | ~b ; break;
        case 14: result =  a |  b ; break;
        case 15: result =  a      ; break;
        }
      reflect_in_flags_zn(result);
      }
    cpu_writereg(A_FIELD(instr), result);
    }
  else if(!instr&0x4000){       // Relative jumps
    }
  else{                         // Miscellaneous
    }
  }
