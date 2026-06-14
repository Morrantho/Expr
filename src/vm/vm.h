#ifndef VM_H
#define VM_H

#include "../base/diag/err.h"
#include "../base/const/const.h"
#include "../base/intern/intern.h"
#include "../compiler/opcode.h"
#include "../compiler/inst.h"
#include "value.h"

#define X_OP_VM_CASE( OP, FN, TK, LHS_TYPE, RHS_TYPE, OUT_TYPE, NAME )\
	case OP_##OP:{ Vm##FN( vm, i ); continue; }

typedef struct Frame { /* literally a span of registers each uses. */
	u8 start;	/* start reg */
	u8 end;		/* end reg ( inclusive ) */
} Frame;

typedef struct Vm {
	Value regs[ CMP_REG_CAP ];
	Frame frames[ CMP_REG_CAP ];
	Interns* interns;
	Consts* consts;
	Funcs* funcs;
	Insts* insts;
	u8 frame; /* current frame */
} Vm;

void VmInit( Vm* vm, Interns* interns, Consts* consts, Funcs* funcs, Insts* insts );
void VmReset( Vm* vm );
void VmPrintValue( Vm* vm, Value* value );
Value* VmRun( Vm* vm, FuncId entry );
// void VmFree( Vm* vm );

#endif