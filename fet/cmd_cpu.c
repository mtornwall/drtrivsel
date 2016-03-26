#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include"fet.h"


static void usage(){
  printf("Usage:  cpu [ reg|flag = <value> ... ]\n");
  }

int cmd_cpu(int again, char **argv){
  int n;
  uword val;
  char *endptr;
  cpu_flags flags;

  if(!argv[1]){
    flags=cpu_read_flags();
    for(int j=0; j<4; j++){
      for(int i=0; i<16; i+=4){
        printf("  %3s: %04X", regname[i+j], (unsigned)cpu_read_reg(i+j));
        }
      if(j==0)printf("     pc: %04X", (unsigned)cpu_read_pc());
      if(j==1)printf("  flags: %c%c%c%c", flags.c?'C':'-', flags.z?'Z':'-',
                                          flags.n?'N':'-', flags.v?'V':'-');
      putchar('\n');
      }
    return 0;
    }

  ++argv;

another:
  if(!argv[1] || strcmp(argv[1], "=") || !argv[2])USAGE()
  
  val=strtoul(argv[2], &endptr, 0);
  if(*endptr)USAGE()

  for(n=0; n<16; ++n)
    if(!strcmp(regname[n], *argv))break;

  if(n<16)                                                // Set GPR
    cpu_write_reg(n, val);
  else if(!strcmp(*argv, "pc"))                           // Set PC
    cpu_write_pc(val);
  else if(!strcmp(*argv, "flags")){                       // Set flag register
printf("set flags\n");
    // ***
    }
  else if((strlen(*argv)==1) && strchr("cznv", **argv)){  // Set flag
printf("set flag %s\n", *argv);
    // ***
    }
  else{
    printf("Unkown register or flag \"%s\"\n", *argv);
    return 1;
    }

  argv+=3;

  if(*argv)goto another;

  return 0;
  }
