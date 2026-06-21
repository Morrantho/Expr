#ifdef TYPES
typedef enum ValueType {
	VALUE_NULL,
	VALUE_NUM,
	VALUE_STR,
} ValueType;

typedef struct Value { /* 4 bytes of waste */
	ValueType type;
	union {
		f64 num;
		InternIdx str;
	};
} Value;

typedef struct Vm {
	Value regs[ REG_CAP ];
	Interns* interns;
	Consts* consts;
	Insts* insts;
	Chunks* chunks;

	Inst *ip, *base, *end;
} Vm;
#endif

#ifdef IMPL
void VmInit( App* app, Vm* vm ){
	vm->interns = &app->interns;
	vm->consts = &app->consts;
	vm->insts = &app->insts;
	vm->chunks = &app->chunks;

	vm->ip = vm->insts->data;
	vm->base = vm->end = 0;
}

static inline void VmEnterChunk( Vm* vm, ChunkIdx idx ){
	Chunk* chunk = ChunkGet( vm->chunks, idx );
	vm->base = vm->insts->data + chunk->start;
	vm->ip = vm->base;
	vm->end = vm->base + chunk->len;
}

static inline Inst* VmInstAt( Vm* vm, InstIdx ip ){
	// if( vm->base + ip >= vm->end ) Halt( ERR_VMIP );
	return vm->base + ip;
}

static inline Value* VmGetValue( Vm* vm, u8 reg ){
	return &vm->regs[ reg ];
}

static inline void VmSetNum( Vm* vm, u8 reg, f64 num ){
	Value* value = VmGetValue( vm, reg );
	value->type = VALUE_NUM;
	value->num = num;
}

static inline f64 VmGetNum( Vm* vm, u8 reg ){
	return VmGetValue( vm, reg )->num;
}

static inline void VmSetX64( Vm* vm, u8 reg, x64 num ){
	VmSetNum( vm, reg, num );
}

static inline x64 VmGetX64( Vm* vm, u8 reg ){
	return ( x64 )VmGetNum( vm, reg );
}

static inline void VmSetStr( Vm* vm, u8 reg, InternIdx str ){
	Value* value = VmGetValue( vm, reg );
	value->type = VALUE_STR;
	value->str = str;
}

static inline InternIdx VmGetStr( Vm* vm, u8 reg ){
	return VmGetValue( vm, reg )->str;
}

static inline x64 VmPowX64( x64 base, x64 exp ){
	if( exp < 0 ) return 0;
	x64 res = 1;
	for( ; exp; exp >>= 1 ){
		if( exp & 1 ) res *= base;
		base *= base;
	}
	return res;
}

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
			printf( "\"%s\"\n", InternGetRaw( vm->interns, value->str ) ); break;
	}
}

#include "vm_ops.h"

Value* VmRun( Vm* vm, ChunkIdx entry ){
	VmEnterChunk( vm, entry );
	for( ;; ){
		Inst* i = vm->ip++;
		switch( ( OpCode )i->op ){
			case OP_ERR: case OP_COUNT: return 0;
			case OP_HALT: return VmGetValue( vm, i->a );
			X_OPS_CORE( X_OP_VM_CASE )
			X_OPS_UNARY( X_OP_VM_CASE )
			X_OPS_POST( X_OP_VM_CASE )
			X_OPS_BINARY( X_OP_VM_CASE )
		}
	}
}
#endif