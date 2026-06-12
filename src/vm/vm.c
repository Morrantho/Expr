#include "vm.h"
#include "ops/ops.h"

void VmInit( Vm* vm, Insts* insts ){
	vm->insts = insts;
}

Reg VmRun( Vm* vm ){
	Inst* ip = vm->insts->code;
	Inst* i;
	for( ;; ){
		i = ip++;
		switch( ( OpCode )i->op ){
			default: return 0;
			X_OPS_CORE( X_OP_VM_CASE )
			// X_OPS_UNA( X_OP_VM_CASE )
			// X_OPS_POST( X_OP_VM_CASE )
			// X_OPS_BIN( X_OP_VM_CASE )
		}
	}
}