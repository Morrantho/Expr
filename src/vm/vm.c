#include "vm.h"

void VmInit( Vm* vm, Interns* interns, Consts* consts, Funcs* funcs, Insts* insts ){
	vm->interns = interns;
	vm->consts = consts;
	vm->funcs = funcs;
	vm->insts = insts;
	vm->frame = 0;
}

void VmReset( Vm* vm ){
	vm->frame = 0;
}

#include "ops/ops.h"

void VmPrintValue( Vm* vm, Value* value ){
	switch( value->type ){
		case VALUE_NULL:
			printf( "NULL\n" ); break;
		case VALUE_NUM:
			printf( "%.15g\n", value->num ); break;
		case VALUE_STR:
			printf( "\"%s\"\n", InternGet( vm->interns, value->str ) ); break;
		case VALUE_FUNC: break;
			// printf( "\n", FuncGet( vm->funcs ) );
	}
}

Value* VmRun( Vm* vm ){
	Inst* ip = vm->insts->code;
	Inst* i;
	for( ;; ){
		i = ip++;
		switch( ( OpCode )i->op ){
			default: case OP_ERR: return 0;
			case OP_HALT: return &vm->regs[ i->a ];
			X_OPS_CORE( X_OP_VM_CASE )
			X_OPS_UNA( X_OP_VM_CASE )
			X_OPS_POST( X_OP_VM_CASE )
			X_OPS_BIN( X_OP_VM_CASE )
		}
	}
}