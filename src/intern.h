#ifdef TYPES
typedef u32 SlotIdx, EntryIdx, InternIdx;

typedef enum HashTag {
	HASH_ID  = 1, /* SOH */
	HASH_STR = 2, /* STX */
} HashTag;

typedef struct Intern {
	Offset offset;
	u32 hash;
	u32 len;
	u8 tag;
} Intern;

typedef struct InternKey {
	HashTag tag;
	u8* src;
	u32 len;
	u32 hash;
} InternKey;

typedef struct Interns {
	Intern* entries;
	EntryIdx* slots; /* stores entry indices */
	Aob aob;
	u32 len, entry_cap, slot_cap, half_cap;
} Interns;
#endif

#ifdef IMPL
u32 HashU8( u32 hash, u8 byte ){
	hash ^= byte;
	hash *= HASH_PRIME;
	return hash;
}

u32 HashStart( HashTag tag ){
	return HashU8( HASH_OFFSET, ( u8 )tag );
}
/* hashes cant = 0. 0 = empty slot. */
u32 HashEnd( u32 hash ){
	return hash + !hash;
}

void InternInit( Interns* interns ){
	AobInit( &interns->aob, INTERN_AOB_CAP );
	interns->entries = MemCalloc( sizeof( interns->entries[ 0 ] ), INTERN_ENTRY_CAP );
	interns->slots = MemCalloc( sizeof( interns->slots[ 0 ] ), INTERN_SLOT_CAP );
	interns->len = 0;
	interns->entry_cap = INTERN_ENTRY_CAP;
	interns->slot_cap = INTERN_SLOT_CAP;
	interns->half_cap = INTERN_SLOT_CAP >> 1;
}

void InternFree( Interns* interns ){
	AobFree( &interns->aob );
	MemFree( interns->slots );
	MemFree( interns->entries );
}

static void InternReput( Interns* interns, EntryIdx entry_idx ){
	Intern* entry = &interns->entries[ entry_idx ];
	u32 mask = interns->slot_cap - 1;
	SlotIdx slot_idx = entry->hash & mask;
	while( interns->slots[ slot_idx ] ){ slot_idx = ( slot_idx + 1 ) & mask; }
	interns->slots[ slot_idx ] = entry_idx + 1;
}

static void InternGrowSlots( Interns* interns ){
	MemFree( interns->slots );
	interns->half_cap = interns->slot_cap; /* always half slot cap */
	interns->slot_cap <<= 1;
	interns->slots = MemCalloc( sizeof( interns->slots[ 0 ] ), interns->slot_cap );
	for( EntryIdx i = 0; i < interns->len; i++ ){ InternReput( interns, i ); }
}

static void InternGrowEntries( Interns* interns ){
	interns->entry_cap <<= 1;
	interns->entries = MemRealloc( interns->entries, sizeof( Intern ), interns->entry_cap );
}

static Offset InternPush( Interns* interns, u8* src, u32 len ){
	Aob* aob = &interns->aob;
	Offset off = AobPush( aob, len + 1 );
	u8* dst = AobGet( aob, off );
	memcpy( dst, src, len );
	dst[ len ] = '\0';
	return off;
}

static SlotIdx InternProbe( Interns* interns, InternKey* key, EntryIdx* out_idx ){
	u32 mask = interns->slot_cap - 1;
	SlotIdx slot_idx = key->hash & mask;
	for( ;; slot_idx = ( slot_idx + 1 ) & mask ){
		EntryIdx entry_idx = interns->slots[ slot_idx ];
		if( !entry_idx ){ *out_idx = 0; return slot_idx; } /* empty slot */
		Intern* entry = &interns->entries[ entry_idx - 1 ];
		if( entry->hash != key->hash || entry->len != key->len || entry->tag != key->tag ){ continue; }
		u8* dst = AobGet( &interns->aob, entry->offset );
		if( memcmp( dst, key->src, key->len ) == 0 ){
			*out_idx = entry_idx;
			return slot_idx;
		}
	}
}

static EntryIdx InternPutInternal( Interns* interns, InternKey* key ){
	EntryIdx entry_idx = 0;
	SlotIdx slot_idx = InternProbe( interns, key, &entry_idx );
	if( entry_idx ){ return entry_idx - 1; } 
	if( interns->len >= interns->entry_cap ){ InternGrowEntries( interns ); }
	if( interns->len >= interns->half_cap ){
		InternGrowSlots( interns );
		slot_idx = InternProbe( interns, key, &entry_idx );
	}
	entry_idx = interns->len++;
	interns->slots[ slot_idx ] = entry_idx + 1; /* Must be non-zero */
	Offset offset = InternPush( interns, key->src, key->len );
	interns->entries[ entry_idx ] = ( Intern ){ offset, key->hash, key->len, key->tag };
	return entry_idx;
}

EntryIdx InternPutId( Interns* interns, u8* src, u32 len, u32 hash ){
	InternKey key = { HASH_ID, src, len, hash };
	return InternPutInternal( interns, &key );
}

EntryIdx InternPutStr( Interns* interns, u8* src, u32 len, u32 hash ){
	InternKey key = { HASH_STR, src, len, hash };
	return InternPutInternal( interns, &key );
}

u8* InternGetRaw( Interns* interns, EntryIdx entry_idx ){
	Intern* entry = &interns->entries[ entry_idx ];
	return AobGet( &interns->aob, entry->offset );
}

InternIdx InternPut( Interns* interns, u8* str ){
	u32 len = 0;
	u32 hash = HashStart( HASH_ID );
	for( ; str[ len ]; len++ ) hash = HashU8( hash, str[ len ] );
	hash = HashEnd( hash );
	return InternPutId( interns, str, len, hash );
}
#endif