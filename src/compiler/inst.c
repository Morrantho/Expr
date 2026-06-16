#include "inst.h"

void InstInit( Insts* insts ){
	insts->code = MemAlloc( sizeof( Inst ), INST_CAP );
	insts->len = 0;
	insts->cap = INST_CAP;
}

void InstReset( Insts* insts ){
	insts->len = 0;
}

static void InstGrow( Insts* insts ){
	insts->cap <<= 1;
	insts->code = MemRealloc( insts->code, sizeof( Inst ), insts->cap );
}

static Inst* InstPush( Insts* insts ){
	if( insts->len >= insts->cap ) InstGrow( insts );
	return &insts->code[ insts->len++ ];
}

void InstABC( Insts* insts, OpCode op, u8 a, u8 b, u8 c ){
	Inst* inst = InstPush( insts );
	inst->op = ( u8 )op;
	inst->a = a;
	inst->b = b;
	inst->c = c;
}

void InstAB( Insts* insts, OpCode op, u8 a, u16 bc ){
	InstABC( insts, op, a, ( u8 )( bc >> 8 ), ( u8 )bc );
}

void InstAppend( Insts* dst, Insts* src, u32 start, u32 end ){
	for( u32 i = start; i < end; i++ ){
		*InstPush( dst ) = src->code[ i ];
	}
}

void InstDump( Insts* insts ){
	for( u32 i = 0; i < insts->len; i++ ){
		Inst* inst = &insts->code[ i ];
		printf( "%s %d %d %d\n", OpGetName( inst->op ), inst->a, inst->b, inst->c );
	}
}

void InstFree( Insts* insts ){
	MemFree( insts->code );
}