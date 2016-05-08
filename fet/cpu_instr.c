#include<assert.h>
#include"fet.h"
#include"cpu_instr.h"


#define CF (cpu_read_flags().c)
#define ZF (cpu_read_flags().z)
#define NF (cpu_read_flags().n)
#define VF (cpu_read_flags().v)


void cpu_init(){
  }

static uword readw_yop(uword instr, int *was_odd){
  uword operand=cpu_read_reg(R_FIELD(instr));
  if(was_odd)*was_odd=1;
  if(L_FIELD(instr))operand+=bus_readw(cpu_read_inc_pc());
  if(I_FIELD(instr)){
    if(was_odd)*was_odd=operand&1;
    operand=bus_readw(operand);
    }
  return operand;
  }

enum{BYTE_TYPE, WORD_TYPE};

static void write_yop(uword instr, uword val, int type){
  uword literal;

  switch(LIR_TYPE(instr)){
    case 0:         // Immediate zero
    case 1: break;  // Register
    case 2:         // Absolute zero (register indirect with %r0)
    case 3:         // Register indirect
      if(type==BYTE_TYPE)bus_writeb(cpu_read_reg(R_FIELD(instr)), val);
      else bus_writew(cpu_read_reg(R_FIELD(instr)), val);
      break;
    case 4:         // Immediate
    case 5: break;  // Register + immediate
    case 6:         // Absolute (indexed with %r0)
    case 7:         // Indexed / Register indirect with displacement
      literal=bus_readw(cpu_read_inc_pc());
      if(type==BYTE_TYPE)bus_writeb(literal+cpu_read_reg(R_FIELD(instr)), val);
      else bus_writew(literal+cpu_read_reg(R_FIELD(instr)), val);
      break;
    }
  }

#define writeb_yop(instr, val) write_yop(instr, val, BYTE_TYPE)
#define writew_yop(instr, val) write_yop(instr, val, WORD_TYPE)

static void reflect_in_flags_zn(uword val){
  cpu_write_zflag(!val);
  cpu_write_nflag(val&0x8000);
  }

void cpu_step(){
  uword instr=bus_readw(cpu_read_inc_pc());

  if(!(instr&0x8000)){          // ALU-op           0 ooooo aaaa l i rrrr
    udword b=readw_yop(instr, 0);
    udword a=cpu_read_reg(A_FIELD(instr));
    udword result;
    if(!(instr&0x4000)){        // Arithmetic       0 0scdf aaaa l i rrrr
      uword carry_in=S_FIELD(instr)?CF:0;
      if(C_FIELD(instr))carry_in=1-carry_in;

      if(F_FIELD(instr)){       // add, adc, inc, b+0, b+c, shl, rol, $1
        result=a+b+carry_in;
        cpu_write_vflag(((a&0x8000) == (b&0x8000)) &&
                        ((a&0x8000) != (result&0x8000)));
        }
      else{                     // sub, sbb, dec, b-0, b-1+c, -c
        result=a-b-1+carry_in;
        cpu_write_vflag(((a&0x8000) != (b&0x8000)) &&
                        ((b&0x8000) == (result&0x8000)));
        }

      cpu_write_cflag(result&0x10000);
      }
    else{                       // Logic            0 1oooo aaaa l i rrrr
      switch(LOGIC_OP(instr)){
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
      }
    result&=0xFFFF;
    reflect_in_flags_zn(result);
    if((instr&0x4000) || !D_FIELD(instr))
      cpu_write_reg(A_FIELD(instr), result);
    }
  else if(!(instr&0x4000)){     // Relative jumps   10 cccc dddddddddd
    int dojump;
    vaddr disp;

    switch(COND_FIELD(instr)){
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
      disp=(disp<<1)&0xFFFF;                // Unit is words, not bytes
      cpu_write_pc(cpu_read_pc()+disp);
      }
    }
  else{                         // Miscellaneous
    uword d, x, reg;
    int was_odd;
    vaddr temp_pc;
    udword dw, result;

    switch(MISC_OP(instr)){
      case  0:                  // 11 0000 aaaa l i rrrr    jmp/here/call/ret
        cpu_write_reg(A_FIELD(instr), cpu_read_pc());
        cpu_write_pc(readw_yop(instr, 0));
        break;

      case  1:                  // 11 0001 aaaa l i rrrr    
        break;

      case  2:                  // 11 0010 aaaa l i rrrr    lbz
      case  3:                  // 11 0011 aaaa l i rrrr    lbs
        d=readw_yop(instr, &was_odd);
        d=was_odd?(d&255):(d>>8);
        if(MISC_OP(instr)&1)d|=(d&128)?0xFF00:0;
        cpu_write_reg(A_FIELD(instr), d);
        reflect_in_flags_zn(d);
        break;

      case  4:                  // 11 0100 aaaa l i rrrr    sw
        writew_yop(instr, cpu_read_reg(A_FIELD(instr)));
        break;

      case  5:                  // 11 0101 aaaa l i rrrr    
        break;

      case  6:                  // 11 0110 aaaa l i rrrr    sb
        writeb_yop(instr, cpu_read_reg(A_FIELD(instr)));
        break;

      case  7:                  // 11 0111 aaaa l i rrrr    
        break;

      case  8:                  // 11 1000 aaaa l i rrrr    push    (a--) <- y
        reg=cpu_read_reg(A_FIELD(instr));
        d=readw_yop(instr, 0);
        bus_writew(reg, d);
        cpu_write_reg(A_FIELD(instr), reg-2);
        reflect_in_flags_zn(d);
        break;

      case  9:                  // 11 1001 aaaa l i rrrr    pop     (++a) -> y
        reg=cpu_read_reg(A_FIELD(instr))+2;
        cpu_write_reg(A_FIELD(instr), reg);
        d=bus_readw(reg);
        writew_yop(instr, d);
        reflect_in_flags_zn(d);
        break;

      case 10:                  // 11 1010 aaaa l i rrrr    xchg
        reg=cpu_read_reg(A_FIELD(instr));
        temp_pc=cpu_read_pc();
        d=readw_yop(instr, 0);
        cpu_write_reg(A_FIELD(instr), d);
        cpu_write_pc(temp_pc);
        writew_yop(instr, reg);
        reflect_in_flags_zn(d);
        break;

      case 11:                  // 11 1011 aaaa l i rrrr    
        break;

      case 12:  // 11 1100 aaaa d t xxxx  logical shift, d selects left/right
                //                        t selects type R/K for "x" operand
        reg=A_FIELD(instr);
        if(SH_T_FIELD(instr))x=cpu_read_reg(SH_X_FIELD(instr));
        else{x=SH_X_FIELD(instr); if(!x)x=16;}
        dw=cpu_read_reg(reg);
        if(SH_D_FIELD(instr)){
          dw=(dw<<1)|CF;
          dw>>=x;
          cpu_write_cflag(dw&1);
          result=dw>>1;
          }
        else{
          dw=dw|(CF<<16);
          dw<<=x;
          cpu_write_cflag(!!(dw&0x10000L));
          result=dw&0xFFFF;
          }
        cpu_write_reg(reg, result);
        reflect_in_flags_zn(result);
        break;

      case 13:  // 11 1101 aaaa d t xxxx  arithmetic shift
        // ***
        break;

      case 14:                  // 11 1110 
        break;

      case 15:                  // 11 1111 
        break;
      }
    }
  }
