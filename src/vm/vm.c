#include "vm.h"

void VmInit( Vm* vm, Interns* interns, Consts* consts, Funcs* funcs, Insts* insts ){
	vm->interns = interns;
	vm->consts = consts;
	vm->funcs = funcs;
	vm->insts = insts;
	vm->ip = 0;
	vm->frame = 0;
	vm->call = 0;
}

void VmReset( Vm* vm ){
	vm->ip = 0;
	vm->frame = 0;
	vm->call = 0;
}

#include "ops/ops.h"

void VmPrintValue( Vm* vm, Value* value ){
	if( !value ){
		printf( "NULL VALUE\n" );
		return;
	}
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

Value* VmRun( Vm* vm, FuncId entry ){
	if( entry == FUNC_NONE ) return 0;
	Func* fn = FuncGet( vm->funcs, entry );
	vm->frame = 0;
	vm->call = 0;
	Frame* frame = &vm->frames[ 0 ];
	frame->start = 0;
	frame->end = fn->nregs;
	vm->ip = &vm->insts->code[ fn->start ];
	for( ;; ){
		Inst* i = vm->ip++;
		switch( ( OpCode )i->op ){
			case OP_ERR: case OP_COUNT: return 0;
			case OP_HALT: return VmGetValue( vm, i->a );
			X_OPS_CORE( X_OP_VM_CASE )
			X_OPS_UNA( X_OP_VM_CASE )
			X_OPS_POST( X_OP_VM_CASE )
			X_OPS_BIN( X_OP_VM_CASE )
		}
	}
}