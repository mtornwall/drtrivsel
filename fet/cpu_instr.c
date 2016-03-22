#include<assert.h>
#include"fet.h"


#define ALU_OP(instr) ((instr&0x7C00)>>10)
#define LOGIC_OP(instr) ((instr&0x3C00)>>10)
#define L_FIELD(instr) ((instr)&0x0020)
#define I_FIELD(instr) ((instr)&0x0010)
#define R_FIELD(instr) ((instr)&0x000F)
#define LIR_FIELD(instr) ((instr)&0x003F)
#define LIR_TYPE(instr) ( (((instr)&0x0030)>>3) | !!((instr)&0x000F) )
#define A_FIELD(instr) (((instr)&0x03C0)>>6)
#define S_FIELD(instr) ((instr)&0x2000)
#define C_FIELD(instr) ((instr)&0x1000)
#define D_FIELD(instr) ((instr)&0x0800)
#define F_FIELD(instr) ((instr)&0x0400)
#define COND_FIELD(instr) ((instr&0x3C00)>>10)
#define DISP_FIELD(instr) ((instr)&0x03FF)
#define DISP(instr) (DISP_FIELD(instr) | ((DISP_FIELD(instr)&0x0200)?0xFC00:0))
#define MISC_OP(instr) ((instr&0x3C00)>>10)
#define SHD_FIELD(instr) ((instr)&0x0020)
#define T_FIELD(instr) ((instr)&0x0010)
#define X_FIELD(instr) ((instr)&0x000F)

// Register names
#define SP 15
#define FP 14
#define RA 13

// L=0, I=0, R=ra
#define RA_LIR RA

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

static void writew_yop(uword instr, uword val){
// ***
  }

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
      cpu_write_pc(cpu_read_pc()+disp);
      }
    }
  else{                         // Miscellaneous
    uword d;
    int was_odd;

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
        break;

      case  4:                  // 11 0100 aaaa l i rrrr    sw
        break;

      case  5:                  // 11 0101 aaaa l i rrrr    
        break;

      case  6:                  // 11 0110 aaaa l i rrrr    sb
        break;

      case  7:                  // 11 0111 aaaa l i rrrr    
        break;

      case  8:                  // 11 1000 aaaa l i rrrr    push    (a--) <- y
        break;

      case  9:                  // 11 1001 aaaa l i rrrr    pop     (++a) -> y
        break;

      case 10:                  // 11 1010 aaaa l i rrrr    xchg
        break;

      case 11:                  // 11 1011 aaaa l i rrrr    
        break;

      case 12:  // 11 1100 aaaa d t xxxx  logical shift, d selects left/right
                //                        t selects type R/K for "x" operand
        break;

      case 13:  // 11 1101 aaaa d t xxxx  arithmetic shift
        break;

      case 14:                  // 11 1110 
        break;

      case 15:                  // 11 1111 
        break;
      }
    }
  }

char *regname[]={"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                 "r8", "r9", "r10", "r11", "r12", "ra", "fp", "sp"};

// Returns 1 or 2, indicating number of words in instruction.
int print_yop(char *to, paddr addr){
  uword instr=bus_readw(addr);
  uword lit;

  if(L_FIELD(instr))lit=bus_readw(addr+2);

  switch(LIR_TYPE(instr)){
    case 0: sprintf(to, "#0"); break;
    case 1: sprintf(to, "%%%s", regname[R_FIELD(instr)]); break;
    case 2: sprintf(to, "0"); break;
    case 3: sprintf(to, "(%%%s)", regname[R_FIELD(instr)]); break;
    case 4: sprintf(to, "#0x%X", (unsigned)lit); break;
    case 5: sprintf(to, "%%%s+0x%X", regname[R_FIELD(instr)],
                                     (unsigned)lit); break;
    case 6: sprintf(to, "0x%04X", (unsigned)lit); break;
    case 7: sprintf(to, "0x%04X(%%%s)", (unsigned)lit,
                                        regname[R_FIELD(instr)]); break;
    }

  return L_FIELD(instr)?2:1;
  }

int disassemble(char *to, paddr addr){
  uword instr=bus_readw(addr);

  if(!(instr&0x8000)){          // ALU-op           0 ooooo aaaa l i rrrr
    int instr_case=(ALU_OP(instr)<<1) | !!LIR_TYPE(instr);
    static char *name[]={
      "acf", "adc", "sbf", "sbb", "acfd", "adcd", "cmpbf", "sbcd",
      "abf", "adb", "scf", "sbc", "abfd", "adbd", "cmpcf", "sbcd",
      "add0", "add", "dec", "subdec", "clctst", "addd", "cmpp1", "subdecd",
      "inc", "addinc", "stctst", "sub", "cmpm1", "addincd", "cmp0", "cmp",
      "rnot", "rnot", "anand", "anand", "rmcl", "rmcl", "clr", "clr",
      "nand", "nand", "not", "not", "xor", "xor", "mcl", "mcl",
      "rmst", "rmst", "axor", "axor", "mov", "mov", "and", "and",
      "set", "set", "mst", "mst", "or", "or", "tst", "tst"
      };
    to+=sprintf(to, "%s %%%s", name[instr_case], regname[A_FIELD(instr)]);
    if(LIR_TYPE(instr)){
      to+=sprintf(to, ", ");
      return print_yop(to, addr);
      }
    return 1;
    }
  else if(!(instr&0x4000)){     // Relative jumps   10 cccc dddddddddd
    vaddr dest=addr+2+DISP(instr);
    static char *j[]={"jlu", "je", "jn", "jv", "jges", "jleu", "jles", "tarp",
                      "jgeu", "jne", "jp", "jnv", "jls", "jgu", "jgs", "j"};
    sprintf(to, "%s 0x%04X", j[(instr&0x3C00)>>10], dest);
    return 1;
    }
  else{                         // Miscellaneous
    switch(MISC_OP(instr)){
      case  0:
        if(!A_FIELD(instr) && LIR_FIELD(instr)==RA_LIR){
          sprintf(to, "ret");
          return 1;
          }
        if(!A_FIELD(instr)){
          to+=sprintf(to, "jmp ");
          return print_yop(to, addr);
          }
        if(A_FIELD(instr) == LIR_FIELD(instr)){
          sprintf(to, "here %%%s", regname[A_FIELD(instr)]);
          return 1;
          }
        to+=sprintf(to, "call ");
        if(A_FIELD(instr)!=RA)
          to+=sprintf(to, "%%%s, ", regname[A_FIELD(instr)]);
        return print_yop(to, addr);
      case 1:
      case 5:
      case 7:
      case 11:
      case 14:
      case 15:
        to+=sprintf(to, "<???""> %%%s, ", regname[A_FIELD(instr)]);
        return print_yop(to, addr);
      case 2:
      case 3:
        to+=sprintf(to, "lb%c %%%s, ", (MISC_OP(instr)==3)?'s':'z',
                                       regname[A_FIELD(instr)]);
        return print_yop(to, addr);
      case 4:
        to+=sprintf(to, "sw %%%s, ", regname[A_FIELD(instr)]);
        return print_yop(to, addr);
      case 6:
        to+=sprintf(to, "sb %%%s, ", regname[A_FIELD(instr)]);
        return print_yop(to, addr);
      case 8:
      case 9:
        to+=sprintf(to, (MISC_OP(instr)==8)?"push ":"pop ");
        if(A_FIELD(instr)!=SP)
          to+=sprintf(to, "%%%s, ", regname[A_FIELD(instr)]);
        return print_yop(to, addr);
      case 10:
        to+=sprintf(to, "xchg %%%s, ", regname[A_FIELD(instr)]);
        return print_yop(to, addr);
      case 12:
      case 13:
        to+=sprintf(to, "%cs%c %%%s, ", (MISC_OP(instr)&1)?'a':'l',
                                        SHD_FIELD(instr)?'r':'l',
                                        regname[A_FIELD(instr)]);
        if(T_FIELD(instr))sprintf(to, "%d", X_FIELD(instr));
        else sprintf(to, "%%%s", regname[X_FIELD(instr)]);
        return 1;
      }
    }
  }

int cmd_dis(int again, char **argv){
  static vaddr next=0;
  static char dis[30];
  int len;

  for(int i=0; i<8; ++i){
    len=disassemble(dis, next);
    printf("%04X: %04X", next, bus_readw(next));
    if(len==2)printf(" %04X", bus_readw(next+2));
    else printf("     ");
    printf("   %s\n", dis);
    next+=len*2;
    }

  return 0;
  }
