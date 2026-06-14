#include "sym.h"

void SymInit( Syms* syms ){
	syms->data = MemAlloc( sizeof( Sym ), SYM_CAP );
	syms->cap = SYM_CAP;
	syms->len = 0;
}

SymId SymGet( Syms* syms, InternId name ){
	for( SymId i = syms->len; i; ){
		i--;
		if( syms->data[ i ].name == name ) return i;
	}
	return SYM_NONE;
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

void SymPutVar( Syms* syms, InternId name, ExprType type, u8 reg ){
	Sym* sym = SymPush( syms, name );
	sym->expr_type = type;
	sym->sym_type = SYM_VAR;
	sym->reg = reg;
}

void SymPutFunc( Syms* syms, InternId name, FuncId funcid ){
	Sym* sym = SymPush( syms, name );
	sym->expr_type = EXPR_FUNC;
	sym->sym_type = SYM_FUNC;
	sym->func = funcid;
}

void SymFree( Syms* syms ){
	MemFree( syms->data );
}