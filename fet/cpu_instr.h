#ifndef CPU_INSTR_H
#define CPU_INSTR_H

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

#endif
