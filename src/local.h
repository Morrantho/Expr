#ifdef TYPES
#define LOCAL_NONE UINT32_MAX
typedef u32 LocalIdx;

typedef struct Local {
	InternIdx name;
	u8 expr_type;
	u8 reg;
} Local;

typedef struct Locals {
	Local* data;
	u32 len;
	u32 cap;
} Locals;
#endif

#ifdef IMPL
void LocalInit( Locals* locals ){
	locals->data = MemAlloc( sizeof( Local ), LOCAL_VEC_CAP );
	locals->cap = LOCAL_VEC_CAP;
	locals->len = 0;
}

void LocalFree( Locals* locals ){
	MemFree( locals->data );
}

Local* LocalFind( Locals* locals, InternIdx name ){
	for( LocalIdx i = locals->len; i; ){
		i--;
		Local* local = &locals->data[ i ];
		if( local->name == name ){ return local; }
	}
	return NULL;
}

static void LocalGrow( Locals* locals ){
	locals->cap <<= 1;
	locals->data = MemRealloc( locals->data, sizeof( Local ), locals->cap );
}

static Local* LocalPush( Locals* locals ){
	if( locals->len >= locals->cap ){ LocalGrow( locals ); }
	return &locals->data[ locals->len++ ];
}

void LocalPut( Locals* locals, InternIdx name, ExprType type, u8 reg ){
	Local* local = LocalPush( locals );
	local->name = name;
	local->expr_type = type;
	local->reg = reg;
}
#endif