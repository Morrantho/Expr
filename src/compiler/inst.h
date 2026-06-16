#ifndef INST_H
#define INST_H

#include "../base/typedefs.h"
#include "../base/cfg.h"
#include "../base/mem/mem.h"
#include "opcode.h"

typedef struct Inst {
	u8 op;
	u8 a, b, c;
} Inst;

typedef struct Insts {
	Inst* code;
	u32 len;
	u32 cap;
} Insts;

void InstInit( Insts* insts );
void InstReset( Insts* insts );
void InstABC( Insts* insts, OpCode op, u8 a, u8 b, u8 c );
void InstAB( Insts* insts, OpCode op, u8 a, u16 bc );
void InstDump( Insts* insts );
void InstFree( Insts* insts );

#endif