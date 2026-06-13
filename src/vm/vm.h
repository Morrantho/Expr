#ifndef VM_H
#define VM_H

#include "../base/const/const.h"
#include "../base/intern/intern.h"
#include "../compiler/opcode.h"
#include "../compiler/inst.h"
#include "value.h"

#define X_OP_VM_CASE( OP, FN, TK, LHS_TYPE, RHS_TYPE, OUT_TYPE, NAME )\
	case OP_##OP:{ Vm##FN( vm, i ); continue; }

typedef struct Vm {
	Interns* interns;
	Consts* consts;
	Insts* insts;
	Value regs[ CMP_REG_CAP ];
} Vm;

void VmInit( Vm* vm, Interns* interns, Consts* consts, Insts* insts );
void VmPrintValue( Vm* vm, Value* value );
Value* VmRun( Vm* vm );
// void VmFree( Vm* vm );

#endif