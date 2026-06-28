#ifdef TYPES
#define FN_NONE UINT32_MAX
typedef u32 FnIdx, ArgIdx;

typedef struct Fn {
	SrcPos body;
	InternIdx name;
	InstIdx entry;
	u32 nregs;
	ArgIdx args;				/* fns->args index */
	u8 nargs;
} Fn;

typedef struct Fns {
	Fn* fns;
	InternIdx* args;
	u32 fn_len, fn_cap;
	u32 arg_len, arg_cap;
} Fns;
#endif

#ifdef IMPL
void FnInit( Fns* fns ){
	fns->fns = MemAlloc( sizeof( Fn ), FN_VEC_CAP );
	fns->args = MemAlloc( sizeof( InternIdx ), REG_CAP );
	fns->fn_len = fns->arg_len = 0;
	fns->fn_cap = FN_VEC_CAP;
	fns->arg_cap = REG_CAP;
}

void FnFree( Fns* fns ){
	MemFree( fns->args );
	MemFree( fns->fns );
}

static void FnArgsGrow( Fns* fns, u32 total ){
	while( total > fns->arg_cap ){ fns->arg_cap <<= 1; }
	fns->args = MemRealloc( fns->args, sizeof( InternIdx ), fns->arg_cap );
}

static void FnGrow( Fns* fns ){
	fns->fn_cap <<= 1;
	fns->fns = MemRealloc( fns->fns, sizeof( Fn ), fns->fn_cap );
}

Fn* FnGet( Fns* fns, FnIdx idx ){
	return &fns->fns[ idx ];
}

InternIdx* FnArgsGet( Fns* fns, Fn* fn ){
	return fns->args + fn->args;
}

FnIdx FnFind( Fns* fns, InternIdx name ){
	for( FnIdx i = fns->fn_len; i; ){
		i--;
		if( fns->fns[ i ].name == name ) return i;
	}
	return FN_NONE;
}

ArgIdx FnArgsPush( Fns* fns, InternIdx* args, u8 nargs ){
	u32 total = fns->arg_len + nargs;
	if( total > fns->arg_cap ){ FnArgsGrow( fns, total ); }
	ArgIdx base = fns->arg_len;
	for( u8 i = 0; i < nargs; i++ )
		{ fns->args[ base + i ] = args[ i ]; }
	fns->arg_len = total;
	return base;
}

FnIdx FnPush( Fns* fns, InternIdx name, SrcPos* body, ArgIdx base, u8 nargs ){
	if( fns->fn_len >= fns->fn_cap ){ FnGrow( fns ); }
	FnIdx fn_idx = fns->fn_len++;
	Fn* fn = FnGet( fns, fn_idx );
	fn->name = name;
	fn->body = *body;
	fn->nregs = 0;
	fn->entry = INST_NONE;
	fn->args = base;
	fn->nargs = nargs;
	return fn_idx;
}
#endif