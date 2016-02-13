#include<assert.h>
#include"fet.h"


#define L_FIELD(instr) ((instr)&0x0020)
#define I_FIELD(instr) ((instr)&0x0010)
#define R_FIELD(instr) ((instr)&0x000F)
#define A_FIELD(instr) (((instr)&0x03C0)>>6)

#define CF (cpu_read_flags().c)
#define ZF (cpu_read_flags().z)
#define NF (cpu_read_flags().n)
#define VF (cpu_read_flags().v)


void cpu_init(){
  }

static uword readw_yop(uword instr){
  uword operand=cpu_read_reg(R_FIELD(instr));
  if(L_FIELD(instr))operand+=bus_readw(cpu_read_inc_pc());
  if(I_FIELD(instr))operand=bus_readw(operand);
  return operand;
  }

static void writew_yop(uword instr, uword val){
// ***
  }

static void reflect_in_flags_zn(uword val){
  cpu_write_zflag(!val);
  cpu_write_nflag(val%0x8000);
  }

void cpu_step(){
  uword instr=bus_readw(cpu_read_inc_pc());

  if(!(instr&0x8000)){          // ALU-op           0 ooooo aaaa l i rrrr
    udword b=readw_yop(instr);
    udword a=cpu_read_reg(A_FIELD(instr));
    udword result;
    if(!(instr&0x4000)){        // Arithmetic       0 0sc.f aaaa l i rrrr
      uword carry_in=(instr&0x2000)?CF:0;
      if(instr&0x1000)carry_in=1-carry_in;

      if(instr&0x0400){
        result=a+b+carry_in;    // add, adc, inc, b+0, b+c, shl, rol, $1
        cpu_write_vflag(((a&0x8000) == (b&0x8000)) &&
                        ((a&0x8000) != (result&0x8000)));
        }
      else{
        result=a-b-1+carry_in;  // sub, sbb, dec, b-0, b-1+c, -c
        cpu_write_vflag(((a&0x8000) != (b&0x8000)) &&
                        ((b&0x8000) == (result&0x8000)));
        }

      cpu_write_cflag(result&0x10000);
      }
    else{                       // Logic            0 1oooo aaaa l i rrrr
      switch((instr&0x3C00)>>10){
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
    cpu_write_reg(A_FIELD(instr), result&0xFFFF);
    }
  else if(!(instr&0x4000)){     // Relative jumps   10 cccc dddddddddd
    int dojump;
    vaddr disp;

    switch((instr&0x3C00)>>10){
      case  0:   dojump=CF;                //  jlu/jc
      case  1:   dojump=ZF;                //  je/jz
      case  2:   dojump=NF;                //  jn/js
      case  3:   dojump=VF;                //  jv
      case  4:   dojump=NF==VF;            //  jges
      case  5:   dojump=CF||ZF;            //  jleu
      case  6:   dojump=ZF||!(NF==VF);     //  jles
      case  7:   dojump=0;                 //  doesn't jump
      case  8:   dojump=!CF;               //  jgeu/jnc
      case  9:   dojump=!ZF;               //  jne/jnz
      case 10:   dojump=!NF;               //  jp/jns
      case 11:   dojump=!VF;               //  jnv
      case 12:   dojump=!(NF==VF);         //  jls
      case 13:   dojump=!(CF||ZF);         //  jgu
      case 14:   dojump=!(ZF||!(NF==VF));  //  jgs
      case 15:   dojump=1;                 //  always jumps
      }
    if(dojump){
      disp=instr&0x3FF;
      if(disp&0x200)disp|=~((vaddr)0x3FF);  // Sign-extend
      cpu_write_pc(cpu_read_pc()+disp);
      }
    }
  else{                         // Miscellaneous
    // ***
    }
  }
