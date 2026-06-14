#include "func.h"

void FuncInit( Funcs* funcs ){
	funcs->data = MemAlloc( sizeof( Func ), FUNC_CAP );
	funcs->len = 0;
	funcs->cap = FUNC_CAP;
}

static void FuncGrow( Funcs* funcs ){
	funcs->cap <<= 1;
	funcs->data = MemRealloc( funcs->data, sizeof( Func ), funcs->cap );
}

FuncId FuncPush( Funcs* funcs ){
	if( funcs->len >= funcs->cap ) FuncGrow( funcs );
	return funcs->len++;
}

Func* FuncGet( Funcs* funcs, FuncId id ){
	return &funcs->data[ id ];
}

void FuncFree( Funcs* funcs ){
	MemFree( funcs->data );
}