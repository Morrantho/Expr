#ifdef TYPES
typedef struct Frame {
	Inst* ip;
	Value* regs;
	Reg ret;
} Frame;

typedef struct Vm {
	Interns* interns;
	Consts* consts;
	Insts* insts;
	Fns* fns;
	Value* regs;
	
	Value reg_stack[ VM_REG_CAP ];
	Frame frames[ VM_FRAME_CAP ];
	u32 nframes;
	Inst *ip;
} Vm;
#endif

#ifdef IMPL
void VmInit( Vm* vm, App* app ){
	vm->interns = &app->interns;
	vm->consts = &app->consts;
	vm->insts = &app->insts;
	vm->fns = &app->fns;
	
	vm->regs = vm->reg_stack;
	vm->ip = vm->insts->data;
	vm->nframes = 0;
}

static Frame* VmFramePush( Vm* vm ){
	if( vm->nframes >= VM_FRAME_CAP ){ Halt( ERR_FRAMEOVERFLOW ); }
	return &vm->frames[ vm->nframes++ ];
}

static inline Frame* VmFramePop( Vm* vm ){
	if( !vm->nframes ){ Halt( ERR_FRAMEUNDERFLOW ); }
	return &vm->frames[ --vm->nframes ];
}

static inline void VmNum( Value* dst, f64 n ){
	dst->type = VALUE_NUM;
	dst->num = n;
}

static inline void VmStr( Value* dst, InternIdx str ){
	dst->type = VALUE_STR;
	dst->str = str;
}

static inline x64 VmPowX64( x64 base, x64 exp ){
	if( exp < 0 ){ return 0; }
	x64 res = 1;
	for( ; exp; exp >>= 1 ){
		if( exp & 1 ){ res *= base; }
		base *= base;
	}
	return res;
}

#include "vm_ops.h"

Value* VmRun( Vm* vm, InstIdx target ){
	vm->regs = vm->reg_stack;
	vm->nframes = 0;
	vm->ip = vm->insts->data + target;
	for( ;; ){
		Inst* i = vm->ip++;
		switch( ( OpCode )i->op ){
			case OP_ERR: case OP_COUNT: return 0;
			case OP_HALT: return &vm->regs[ i->a ];
			X_OPS_CORE( X_OP_VM_CORE_CASE )
			X_OPS_UNARY( X_OP_VM_UNARY_CASE )
			X_OPS_POST( X_OP_VM_POST_CASE )
			X_OPS_BINARY( X_OP_VM_BINARY_CASE )
		}
	}
}
#endif