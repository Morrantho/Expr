#ifdef TYPES
typedef u32 InstIdx;

typedef struct Inst {
	u8 op, a, b, c;
} Inst;

typedef struct Insts {
	Inst* data;
	u32 len, cap;
} Insts;
#endif

#ifdef IMPL
void InstInit( Insts* insts ){
	insts->data = MemAlloc( sizeof( insts->data[ 0 ] ), INST_VEC_CAP );
	insts->len = 0;
	insts->cap = INST_VEC_CAP;
}

void InstFree( Insts* insts ){
	MemFree( insts->data );
	insts->data = 0;
	insts->len = insts->cap = 0;
}

static void InstGrow( Insts* insts ){
	insts->cap <<= 1;
	insts->data = MemRealloc( insts->data, sizeof( insts->data[ 0 ] ), insts->cap );
}

static Inst* InstPush( Insts* insts, InstIdx* out ){
	if( insts->len >= insts->cap ){ InstGrow( insts ); }
	*out = insts->len++;
	return &insts->data[ *out ];
}

static Inst* InstGet( Insts* insts, InstIdx idx ){
	return &insts->data[ idx ];
}

void InstDump( Insts* insts ){
	for( u32 i = 0; i < insts->len; i++ ){
		Inst* inst = &insts->data[ i ];
		printf( "%d: %s r%d %d %d\n", i, OpGetName( inst->op ), inst->a, inst->b, inst->c );
	}
}

static InstIdx InstABC( Insts* insts, OpCode op, u8 a, u8 b, u8 c ){
	InstIdx idx = UINT32_MAX;
	*InstPush( insts, &idx ) = ( Inst ){ op, a, b, c };
	return idx;
}

static InstIdx InstABX( Insts* insts, OpCode op, u8 a, u16 bx ){
	return InstABC( insts, op, a, bx >> 8, bx );
}

// static InstIdx InstAX( Insts* insts, OpCode op, u32 ax ){
// 	return InstABC( insts, op, ax >> 16, ax >> 8, ax );
// }

static u16 InstGetBX( Inst* inst ){
	return ( ( u16 )inst->b << 8 ) | inst->c;
}

// static u32 InstGetAX( Inst* inst ){
// 	return ( ( u32 )inst->a << 16 ) | ( ( u32 )inst->b << 8 ) | inst->c;
// }

// static InstIdx InstMov( Insts* insts, u8 dst, u8 src ){
// 	return InstABC( insts, OP_MOV, dst, src, 0 );
// }

static InstIdx InstJmp( Insts* insts, u16 bx ){
	return InstABX( insts, OP_JMP, 0, bx );
}

static InstIdx InstJz(  Insts* insts, u8 reg ){
	return InstABX( insts, OP_JZ, reg, 0 );
}

// static InstIdx InstJnz(  Insts* insts, u8 reg ){
// 	return InstABX( insts, OP_JNZ, reg, 0 );
// }
#endif