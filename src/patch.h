#ifdef TYPES
typedef u32 PatchIdx;

typedef enum PatchType {
	PATCH_CONTINUE,
	PATCH_BREAK,
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
void PatchBX( Insts* insts, InstIdx idx, InstIdx bx ){
	// if( bx > UINT16_MAX ) Halt( ERR_INSTPATCH );
	Inst* inst = InstGet( insts, idx );
	inst->b = bx >> 8;
	inst->c = bx;
}

void PatchInit( App* app, Patches* patches ){
	patches->insts = &app->insts;
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
	if( patches->len >= patches->cap ) PatchGrow( patches );
	patches->data[ patches->len++ ] = ( Patch ){
		.type = type,
		.inst = idx
	};
}

void PatchApply( Patches* patches, PatchType type, PatchIdx mark, InstIdx target ){
	for( ; mark < patches->len; mark++ ){
		Patch* patch = &patches->data[ mark ];
		if( patch->type != type ) continue;
		PatchBX( patches->insts, patch->inst, target );
	}
}
#endif