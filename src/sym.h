#ifdef TYPES
#define SYM_NONE UINT32_MAX
typedef u32 SymIdx;

typedef struct Sym {
	InternIdx name;
	u8 expr_type;
	u8 reg;
} Sym;

typedef struct Syms {
	Sym* data;
	u32 len;
	u32 cap;
} Syms;
#endif

#ifdef IMPL
void SymInit( Syms* syms ){
	syms->data = MemAlloc( sizeof( Sym ), SYM_VEC_CAP );
	syms->cap = SYM_VEC_CAP;
	syms->len = 0;
}

void SymFree( Syms* syms ){
	MemFree( syms->data );
}

Sym* SymGet( Syms* syms, InternIdx name ){
	for( SymIdx i = syms->len; i; ){
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

static Sym* SymPush( Syms* syms ){
	if( syms->len >= syms->cap ) SymGrow( syms );
	return &syms->data[ syms->len++ ];
}

void SymPut( Syms* syms, InternIdx name, ExprType type, u8 reg ){
	Sym* sym = SymPush( syms );
	sym->name = name;
	sym->expr_type = type;
	sym->reg = reg;
}
#endif