#ifdef TYPES
typedef u32 ConstIdx;

typedef enum ConstType {
	CONST_NUM,
	CONST_STR,
} ConstType;

typedef struct Const{
	ConstType type;
	union {
		f64 num;
		InternIdx str;
	};
} Const;

typedef struct Consts {
	Const* data;
	u32 len, cap;
} Consts;

static ConstIdx CONST_VOID = 0;
#endif

#ifdef IMPL
ConstIdx ConstPutNum( Consts* consts, f64 num );

void ConstInit( Consts* consts ){
	consts->data = MemAlloc( sizeof( consts->data[ 0 ] ), CONST_VEC_CAP );
	consts->len = 0;
	consts->cap = CONST_VEC_CAP;
	CONST_VOID = ConstPutNum( consts, 0 );
	// if( CONST_VOID != 0 ) Halt( ERR_CONSTVOID );
}

static void ConstGrow( Consts* consts ){
	consts->cap <<= 1;
	consts->data = MemRealloc( consts->data, sizeof( consts->data[ 0 ] ), consts->cap );
}

static ConstIdx ConstPut( Consts* consts, ConstType type ){
	if( consts->len >= consts->cap ) ConstGrow( consts );
	ConstIdx idx = consts->len++;
	consts->data[ idx ] = ( Const ){ .type = type };
	return idx;
}

ConstIdx ConstPutNum( Consts* consts, f64 num ){
	ConstIdx idx = ConstPut( consts, CONST_NUM );
	consts->data[ idx ].num = num;
	return idx;
}

ConstIdx ConstPutStr( Consts* consts, InternIdx str ){
	ConstIdx idx = ConstPut( consts, CONST_STR );
	consts->data[ idx ].str = str;
	return idx;
}

Const* ConstGet( Consts* consts, ConstIdx idx ){
	return &consts->data[ idx ];
}

void ConstFree( Consts* consts ){
	MemFree( consts->data );
}
#endif