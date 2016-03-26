#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include"fet.h"
#include"cpu_instr.h"


char *regname[]={"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                 "r8", "r9", "r10", "r11", "r12", "ra", "fp", "sp"};

// Returns 1 or 2, indicating number of words in instruction.
static int print_yop(char *to, paddr addr){
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

static int disassemble(char *to, paddr addr){
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
    sprintf(to, "%s 0x%04X", j[(instr&0x3C00)>>10], (unsigned)dest);
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
  return 1;
  }

static void usage(){
  printf("Usage:  dis [addr]\n");
  }

int cmd_dis(int again, char **argv){
  static vaddr next=0;
  vaddr potential;
  char *endptr;
  int print_next=0;
  static char dis[30];
  int len;
  int lines=8;

  if(!again){
    if(argv[1] && !strcmp(argv[1], "-n")){
      print_next=1;
      lines=1;
      ++argv;
      }
    if(argv[1]){
      potential=strtoul(argv[1], &endptr, 0);
      if(*endptr)USAGE()
      next=potential;
      }
    }

  for(int i=0; i<lines; ++i){
    len=disassemble(dis, next);
    if(catch_bus_error())return 1;

    if(print_next)printf("next: ");
    printf("%04X: %04X", (unsigned)next, (unsigned)bus_readw(next));
    if(len==2)printf(" %04X", (unsigned)bus_readw(next+2));
    else printf("     ");
    printf("   %s\n", dis);

    next+=len*2;
    }

  return 0;
  }
