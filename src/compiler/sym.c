#include "sym.h"

void SymInit( Syms* syms ){
	syms->data = MemAlloc( sizeof( Sym ), SYM_CAP );
	syms->cap = SYM_CAP;
	syms->len = 0;
}

Sym* SymGet( Syms* syms, InternId name ){
	for( SymId i = syms->len; i; ){
		i--;
		Sym* sym = &syms->data[ i ];
		if( sym->name == name ) return sym;
	}
	return NULL;
}

static void SymGrow( Syms* syms ){
	syms->cap <<= 1;
	syms->data = MemRealloc( syms->data, sizeof( Sym ), syms->cap );
}

static Sym* SymPush( Syms* syms, InternId name ){
	if( syms->len >= syms->cap ) SymGrow( syms );
	Sym* sym = &syms->data[ syms->len++ ];
	sym->name = name;
	return sym;
}

void SymPut( Syms* syms, InternId name, ExprType type, u8 reg ){
	Sym* sym = SymPush( syms, name );
	sym->expr_type = type;
	sym->reg = reg;
}

void SymFree( Syms* syms ){
	MemFree( syms->data );
}