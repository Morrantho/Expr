#ifdef TYPES
#define FN_DECL_NONE UINT32_MAX
#define FN_IMPL_NONE UINT32_MAX
typedef u32 FnDeclIdx, FnImplIdx, FnArgIdx, FnTypeIdx;

typedef struct FnDecl {
	InternIdx name;
	SrcSpan body;
	FnArgIdx arg_base;				/* fns->args index */
	u8 nargs;
} FnDecl;

typedef struct FnImpl {
	FnDeclIdx decl;
	ChunkIdx chunk;
	FnTypeIdx type_base; 			/* fns->types index */
} FnImpl;

typedef struct Fns {
	FnDecl* decls;
	InternIdx* args;				/* decl arg names */
	FnImpl* impls;
	ExprType* types;				/* impl arg types */
	u32 decl_len, decl_cap;
	u32 arg_len, arg_cap;
	u32 impl_len, impl_cap;
	u32 type_len, type_cap;
} Fns;
#endif

#ifdef IMPL
void FnInit( Fns* fns ){
	fns->decls = MemAlloc( sizeof( FnDecl ), FN_DECL_VEC_CAP );
	fns->args = MemAlloc( sizeof( InternIdx ), REG_CAP );
	fns->impls = MemAlloc( sizeof( FnImpl ), FN_IMPL_VEC_CAP );
	fns->types = MemAlloc( sizeof( ExprType ), FN_TYPE_VEC_CAP );
	fns->decl_len = fns->arg_len = fns->impl_len = fns->type_len = 0;
	fns->decl_cap = FN_DECL_VEC_CAP;
	fns->arg_cap = REG_CAP;
	fns->impl_cap = FN_IMPL_VEC_CAP;
	fns->type_cap = FN_TYPE_VEC_CAP;
}

void FnFree( Fns* fns ){
	MemFree( fns->types );
	MemFree( fns->impls );
	MemFree( fns->args );
	MemFree( fns->decls );
}

static void FnArgsGrow( Fns* fns, u32 total ){
	while( total > fns->arg_cap ) fns->arg_cap <<= 1;
	fns->args = MemRealloc( fns->args, sizeof( InternIdx ), fns->arg_cap );
}

static void FnDeclGrow( Fns* fns ){
	fns->decl_cap <<= 1;
	fns->decls = MemRealloc( fns->decls, sizeof( FnDecl ), fns->decl_cap );
}

static void FnTypesGrow( Fns* fns, u32 total ){
	while( total > fns->type_cap ) fns->type_cap <<= 1;
	fns->types = MemRealloc( fns->types, sizeof( ExprType ), fns->type_cap );
}

static void FnImplGrow( Fns* fns ){
	fns->impl_cap <<= 1;
	fns->impls = MemRealloc( fns->impls, sizeof( FnImpl ), fns->impl_cap );
}

FnDecl* FnDeclGet( Fns* fns, FnDeclIdx idx ){
	return &fns->decls[ idx ];
}

FnImpl* FnImplGet( Fns* fns, FnImplIdx idx ){
	return &fns->impls[ idx ];
}

InternIdx* FnArgsGet( Fns* fns, FnDecl* decl ){
	return fns->args + decl->arg_base;
}

ExprType* FnTypesGet( Fns* fns, FnImpl* impl ){
	return fns->types + impl->type_base;
}

FnArgIdx FnArgsPush( Fns* fns, InternIdx* args, u8 nargs ){
	u32 total = fns->arg_len + nargs;
	if( total > fns->arg_cap ) FnArgsGrow( fns, total );
	FnArgIdx base = fns->arg_len;
	for( u8 i = 0; i < nargs; i++ ) fns->args[ base + i ] = args[ i ];
	fns->arg_len = total;
	return base;
}

FnDeclIdx FnDeclPush( Fns* fns, InternIdx name, SrcSpan* body, FnArgIdx base, u8 nargs ){
	if( fns->decl_len >= fns->decl_cap ) FnDeclGrow( fns );
	FnDeclIdx decl_idx = fns->decl_len++;
	FnDecl* decl = FnDeclGet( fns, decl_idx );
	decl->name = name;
	decl->body = *body;
	decl->arg_base = base;
	decl->nargs = nargs;
	return decl_idx;
}

FnTypeIdx FnTypesPush( Fns* fns, ExprType* types, u8 nargs ){
	u32 total = fns->type_len + nargs;
	if( total > fns->type_cap ) FnTypesGrow( fns, total );
	FnTypeIdx base = fns->type_len;
	for( u8 i = 0; i < nargs; i++ ) fns->types[ base + i ] = types[ i ];
	fns->type_len = total;
	return base;
}

FnImplIdx FnImplPush( Fns* fns, FnDeclIdx decl, ChunkIdx chunk, FnTypeIdx base ){
	if( fns->impl_len >= fns->impl_cap ) FnImplGrow( fns );
	FnImplIdx impl_idx = fns->impl_len++;
	FnImpl* impl = FnImplGet( fns, impl_idx );
	impl->decl = decl;
	impl->chunk = chunk;
	impl->type_base = base;
	return impl_idx;
}
#endif