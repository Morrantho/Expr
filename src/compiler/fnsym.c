#include "fnsym.h"

void FnSymInit( FnSyms* syms ){
	syms->data = MemAlloc( sizeof( FnSym ), SYM_CAP );
	syms->len = 0;
	syms->cap = SYM_CAP;
}

static void FnSymGrow( FnSyms* syms ){
	syms->cap <<= 1;
	syms->data = MemRealloc( syms->data, sizeof( FnSym ), syms->cap );
}

FnSym* FnSymGet( FnSyms* syms, InternId name ){
	for( u32 i = syms->len; i; ){
		i--;
		FnSym* sym = &syms->data[ i ];
		if( sym->name == name ) return sym;
	}
	return NULL;
}

void FnSymPut( FnSyms* syms, InternId name, FuncId fn_id ){
	if( syms->len >= syms->cap ) FnSymGrow( syms );
	FnSym* sym = &syms->data[ syms->len++ ];
	sym->name = name;
	sym->fn_id = fn_id;
}

void FnSymFree( FnSyms* syms ){
	MemFree( syms->data );
}
