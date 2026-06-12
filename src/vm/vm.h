#ifndef VM_H
#define VM_H

#include "../compiler/opcode.h"
#include "../compiler/inst.h"

#define X_OP_VM_CASE( OP, FN, TK, LHS_TYPE, RHS_TYPE, OUT_TYPE, NAME )\
case OP_##OP:{ Vm##FN( vm, i ); continue; }

typedef struct Vm {
	Insts* insts;
} Vm;

void VmInit( Vm* vm, Insts* insts );
Reg VmRun( Vm* vm );
// void VmFree( Vm* vm );

#endif