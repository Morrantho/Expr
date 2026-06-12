#include "vm.h"

void VmInit( Vm* vm, Insts* insts ){
	vm->insts = insts;
}

Reg VmRun( Vm* vm ){
	Inst* ip = vm->insts->code;
	Inst* i;
	for( ;; ){
		i = ip++;
		switch( ( OpCode )i->op ){
			case OP_HALT: return i->a;
			default:{
				u8* op_name = OpGetName( i->op );
				printf( "%s %d %d %d\n", op_name, i->a, i->b, i->c );
				continue;
			}
		}
	}
}