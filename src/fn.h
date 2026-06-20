#ifdef TYPES
#define FN_NONE UINT16_MAX
typedef u16 FnIdx;

typedef struct Fn {
	u32 start;	/* insts start index */
	u32 end;	/* insts end index */
	u8 nargs;
	u8 nregs;
	ExprType ret_type;
} Fn;

typedef struct Fns {
	Fn* data;
	u32 len, cap;
} Fns;
#endif

#ifdef IMPL
void FnInit( Fns* fns ){
	fns->data = MemAlloc( sizeof( Fn ), FN_VEC_CAP );
	fns->len = 0;
	fns->cap = FN_VEC_CAP;
}

static void FnGrow( Fns* fns ){
	fns->cap <<= 1;
	fns->data = MemRealloc( fns->data, sizeof( Fn ), fns->cap );
}

FnIdx FnPush( Fns* fns ){
	if( fns->len >= fns->cap ) FnGrow( fns );
	return fns->len++;
}

Fn* FnGet( Fns* fns, FnIdx idx ){
	return &fns->data[ idx ];
}

void FnFree( Fns* fns ){
	MemFree( fns->data );
}
#endif