#ifdef TYPES
typedef u32 PatchIdx;

typedef enum PatchType {
	PATCH_BREAK,
	PATCH_CONTINUE,
	PATCH_BRANCH,	/* try next branch */
	PATCH_END,		/* end of if chain */
} PatchType;

typedef struct Patch {
	PatchType type;
	InstIdx inst;
} Patch;

typedef struct Patches {
	Insts* insts;
	Patch* data;
	u32 len;
	u32 cap;
} Patches;
#endif

#ifdef IMPL
void PatchBX( Inst* dst, InstIdx bx ){
	// if( bx > UINT16_MAX ) Halt( ERR_INSTPATCH );
	dst->b = bx >> 8;
	dst->c = bx;
}

void PatchInit( Patches* patches, Insts* insts ){
	patches->insts = insts;
	patches->data = MemAlloc( sizeof( patches->data[ 0 ] ), PATCH_VEC_CAP );
	patches->len = 0;
	patches->cap = PATCH_VEC_CAP;
}

void PatchFree( Patches* patches ){
	MemFree( patches->data );
}

static void PatchGrow( Patches* patches ){
	patches->cap <<= 1;
	patches->data = MemRealloc( patches->data, sizeof( patches->data[ 0 ] ), patches->cap );
}

void PatchReset( Patches* patches, PatchIdx mark ){
	patches->len = mark;
}

void PatchPush( Patches* patches, PatchType type, InstIdx idx ){
	if( patches->len >= patches->cap ){ PatchGrow( patches ); }
	patches->data[ patches->len++ ] = ( Patch ){
		.type = type,
		.inst = idx
	};
}

void PatchApply( Patches* patches, PatchType type, PatchIdx mark, InstIdx target ){
	for( ; mark < patches->len; mark++ ){
		Patch* patch = &patches->data[ mark ];
		if( patch->type != type ){ continue; }
		Inst* inst = InstGet( patches->insts, patch->inst );
		PatchBX( inst, target );
	}
	PatchReset( patches, mark );
}

/* so we dont do 2 separate passes for no reason. */
void PatchLoop( Patches* patches, PatchIdx mark, InstIdx cont, InstIdx brk ){
	for( ; mark < patches->len; mark++ ){
		Patch* patch = &patches->data[ mark ];
		Inst* inst = InstGet( patches->insts, patch->inst );
		switch( patch->type ){
			default: continue;
			case PATCH_BREAK: PatchBX( inst, brk ); break;
			case PATCH_CONTINUE: PatchBX( inst, cont ); break;
		}
	}
	PatchReset( patches, mark );
}
#endif