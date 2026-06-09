#include "const.h"

void ConstInit( Consts* consts ){
	consts->items = MemAlloc( sizeof( Const ), CONST_CAP );
	consts->len = 0;
	consts->cap = CONST_CAP;
}

static void ConstGrow( Consts* consts ){
	consts->cap <<= 1;
	consts->items = MemRealloc( consts->items, sizeof( Const ), consts->cap );
}

static ConstId ConstPut( Consts* consts, ConstType type ){
	if( consts->len >= consts->cap ) ConstGrow( consts );
	ConstId idx = consts->len++;
	consts->items[ idx ] = ( Const ){ .type = type };
	return idx;
}

ConstId ConstPutNum( Consts* consts, f64 num ){
	ConstId idx = ConstPut( consts, CONST_NUM );
	consts->items[ idx ].num = num;
	return idx;
}

ConstId ConstPutStr( Consts* consts, InternId str ){
	ConstId idx = ConstPut( consts, CONST_STR );
	consts->items[ idx ].str = str;
	return idx;
}

Const* ConstGet( Consts* consts, ConstId idx ){
	return &consts->items[ idx ];
}

void ConstFree( Consts* consts ){
	MemFree( consts->items );
}