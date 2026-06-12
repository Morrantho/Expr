#ifndef VM_H
#define VM_H

#include "../compiler/inst.h"

typedef struct Vm {
	Insts* insts;
} Vm;

void VmInit( Vm* vm, Insts* insts );
Reg VmRun( Vm* vm );
// void VmFree( Vm* vm );

#endif