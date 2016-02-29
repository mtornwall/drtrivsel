#include<assert.h>
#include"fet.h"


#define L_FIELD(instr) ((instr)&0x0020)
#define I_FIELD(instr) ((instr)&0x0010)
#define R_FIELD(instr) ((instr)&0x000F)
#define LIR_TYPE(instr) ( (((instr)&0x0030)>>3) | !!((instr)&0x000F) )
#define A_FIELD(instr) (((instr)&0x03C0)>>6)
#define D_FIELD(instr) ((instr)&0x03FF)
#define DISP(instr) ( D_FIELD(instr) | ((D_FIELD(instr)&0x0200)?0xFC00:0) )

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

// Returns 1 or 2, indicating number of words in instruction.
int print_yop(char *to, paddr addr){
  uword instr=bus_readw(addr);
  uword lit;

  if(L_FIELD(instr))lit=bus_readw(addr+2);

  switch(LIR_TYPE(instr)){
    case 0: sprintf(to, "#0"); break;
    case 1: sprintf(to, "%%r%d", R_FIELD(instr)); break;
    case 2: sprintf(to, "0"); break;
    case 3: sprintf(to, "(%%r%d)", R_FIELD(instr)); break;
    case 4: sprintf(to, "#0x%X", (unsigned)lit); break;
    case 5: sprintf(to, "%%r%d+0x%X", R_FIELD(instr), (unsigned)lit); break;
    case 6: sprintf(to, "0x%04X", (unsigned)lit); break;
    case 7: sprintf(to, "0x%04X(%%r%d)", (unsigned)lit, R_FIELD(instr); break;
    }

  return L_FIELD(instr)?2:1;
  }

int disassemble(char *to, paddr addr){
  uword instr=bus_readw(addr);

  if(!(instr&0x8000)){          // ALU-op           0 ooooo aaaa l i rrrr
    //A_FIELD(instr);
    switch((instr&0x7C00)>>10){
        // add, adc, inc, b+0, b+c, shl, rol, $1
        // sub, sbb, dec, b-0, b-1+c, -c
      case 0x00:  to+=sprintf(to, "sub"); break; // c0  sub
      case 0x01:  to+=sprintf(to, ""); break; // c0  add
      case 0x04:  to+=sprintf(to, ""); break; // c1  sub
      case 0x05:  to+=sprintf(to, ""); break; // c1  add
      case 0x08:  to+=sprintf(to, ""); break; // cin sub
      case 0x09:  to+=sprintf(to, ""); break; // cin add
      case 0x0C:  to+=sprintf(to, ""); break; // cin inv sub
      case 0x0D:  to+=sprintf(to, ""); break; // cin inv add

      case 0x10:  to+=sprintf(to, ""); break; // ~a       rnot
      case 0x11:  to+=sprintf(to, ""); break; // ~a | ~b  yanand
      case 0x12:  to+=sprintf(to, ""); break; // ~a &  b  rmcl
      case 0x13:  to+=sprintf(to, ""); break; //  0       clr
      case 0x14:  to+=sprintf(to, ""); break; // ~(a & b) nand
      case 0x15:  to+=sprintf(to, ""); break; //      ~b  not
      case 0x16:  to+=sprintf(to, ""); break; //  a ^  b  xor
      case 0x17:  to+=sprintf(to, ""); break; //  a & ~b  mcl
      case 0x18:  to+=sprintf(to, ""); break; // ~a |  b  rmst
      case 0x19:  to+=sprintf(to, ""); break; // ~a ^ ~b  yaxor
      case 0x1A:  to+=sprintf(to, ""); break; //       b  mov
      case 0x1B:  to+=sprintf(to, ""); break; //  a &  b  and
      case 0x1C:  to+=sprintf(to, ""); break; // ~0       set
      case 0x1D:  to+=sprintf(to, ""); break; //  a | ~b  mst
      case 0x1E:  to+=sprintf(to, ""); break; //  a |  b  or
      case 0x1F:  to+=sprintf(to, ""); break; //  a       tst
      }
    to+=sprintf(to, "%%r%d, ", A_FIELD(instr));
    return print_yop(to, addr);
    }
  else if(!(instr&0x4000)){     // Relative jumps   10 cccc dddddddddd
    vaddr dest=addr+DISP(instr);
    static char *j[]={"jlu", "je", "jn", "jv", "jges", "jleu", "jles", "tarp",
                      "jgeu", "jne", "jp", "jnv", "jls", "jgu", "jgs", "j"};
    sprintf(to, "%s %04X", j[(instr&0x3C00)>>10], dest);
    return 1;
    }
  else{                         // Miscellaneous
    // ***
    sprintf(to, "?");
    return 1;
    }
  }

/*
scdfr
00000          cf add  #0  acf %r
00001          cf add   y  adc %r, y
00010          cf  sub #0  sbf %r
00011          cf  sub  y  sbb %r, y
00100 discard  cf add  #0  acfd %r
00101 discard  cf add   y  adcd %r, y
00110 discard  cf  sub #0  cmpbf %r
00111 discard  cf  sub  y  sbcd %r, y
01000         ~cf add  #0  abf  %r
01001         ~cf add   y  adb  %r, y
01010         ~cf  sub #0  scf  %r
01011         ~cf  sub  y  sbc  %r, y
01100 discard ~cf add  #0  abfd %r
01101 discard ~cf add   y  adbd %r, y
01110 discard ~cf  sub #0  cmpcf %r
01111 discard ~cf  sub  y  sbcd %r, y
10000          c0 add  #0  add %r, #0
10001          c0 add   y  add %r, y
10010          c0  sub #0  dec %r
10011          c0  sub  y  subdec %r, y
10100 discard  c0 add  #0  clcish
10101 discard  c0 add   y  addd
10110 discard  c0  sub #0  cmpp1
10111 discard  c0  sub  y  subdecd
11000          c1 add  #0  inc
11001          c1 add   y  addinc
11010          c1  sub #0  stcish
11011          c1  sub  y  sub
11100 discard  c1 add  #0  cmpn1
11101 discard  c1 add   y  addincd
11110 discard  c1  sub #0  cmp0
11111 discard  c1  sub  y  cmp
*/
