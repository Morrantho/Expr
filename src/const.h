#ifdef TYPES
typedef u32 ConstIdx;

typedef struct Consts {
	Value* data;
	u32 len, cap;
} Consts;

static ConstIdx CONST_VOID = 0;
#endif

#ifdef IMPL
ConstIdx ConstPut( Consts* consts, Value value );

void ConstInit( Consts* consts ){
	consts->data = MemAlloc( sizeof( consts->data[ 0 ] ), CONST_VEC_CAP );
	consts->len = 0;
	consts->cap = CONST_VEC_CAP;
	CONST_VOID = ConstPut( consts, ValueNull( ) );
	// if( CONST_VOID != 0 ) Halt( ERR_CONSTVOID );
}

static void ConstGrow( Consts* consts ){
	consts->cap <<= 1;
	consts->data = MemRealloc( consts->data, sizeof( consts->data[ 0 ] ), consts->cap );
}

ConstIdx ConstPut( Consts* consts, Value value ){
	if( consts->len >= consts->cap ){ ConstGrow( consts ); }
	ConstIdx idx = consts->len++;
	consts->data[ idx ] = value;
	return idx;
}

ConstIdx ConstPutNum( Consts* consts, f64 num ){
	return ConstPut( consts, ValueNum( num ) );
}

ConstIdx ConstPutStr( Consts* consts, InternIdx str ){
	return ConstPut( consts, ValueStr( str ) );
}

Value* ConstGet( Consts* consts, ConstIdx idx ){
	return &consts->data[ idx ];
}

void ConstFree( Consts* consts ){
	MemFree( consts->data );
}
#endif