#include "intern.h"

void InternInit( Interns* interns ){
	interns->entry_cap = INTERN_CAP;
	interns->slot_cap = INTERN_CAP << 1;
	interns->slot_half_cap = INTERN_CAP;
	interns->slots = MemCalloc( sizeof( u32 ), interns->slot_cap );
	interns->entries = MemCalloc( sizeof( Intern ), interns->entry_cap );
	AobInit( &interns->aob, INTERN_AOB_CAP );
	interns->len = 0;
}

static u32 InternProbe( Interns* interns, InternKey* key, InternId* out ){
	u32 mask = interns->slot_cap - 1;
	u32 slot_idx = key->hash & mask;
	for( ;; slot_idx = ( slot_idx + 1 ) & mask ){
		u32 entry_idx = interns->slots[ slot_idx ];
		if( !entry_idx ){ *out = 0; return slot_idx; } /* empty slot */
		Intern* entry = &interns->entries[ entry_idx - 1 ];
		if( entry->hash != key->hash || entry->len != key->len || entry->tag != key->tag ) continue;
		u8* dst = AobGet( &interns->aob, entry->offset );
		if( memcmp( dst, key->src, key->len ) == 0 ){
			*out = entry_idx;
			return slot_idx;
		}
	}
}

static Offset InternPush( Interns* interns, u8* src, u32 len ){
	Aob* aob = &interns->aob;
	Offset off = AobPush( aob, len + 1 );
	u8* dst = AobGet( aob, off );
	memcpy( dst, src, len );
	dst[ len ] = '\0';
	return off;
}

static void InternReput( Interns* interns, InternId id ){
	Intern* entry = &interns->entries[ id ];
	u32 mask = interns->slot_cap - 1;
	u32 slot_idx = entry->hash & mask;
	while( interns->slots[ slot_idx ] ) slot_idx = ( slot_idx + 1 ) & mask;
	interns->slots[ slot_idx ] = id + 1;
}

static void InternGrowSlots( Interns* interns ){
	MemFree( interns->slots );
	interns->slot_cap <<= 1;
	interns->slot_half_cap = interns->slot_cap >> 1;
	interns->slots = MemCalloc( sizeof( u32 ), interns->slot_cap );
	for( InternId id = 0; id < interns->len; id++ ) InternReput( interns, id );
}

static void InternGrowEntries( Interns* interns ){
	interns->entry_cap <<= 1;
	interns->entries = MemRealloc( interns->entries, sizeof( Intern ), interns->entry_cap );
}

static InternId InternPutInternal( Interns* interns, InternKey* key ){
	InternId entry_idx = 0;
	u32 slot_idx = InternProbe( interns, key, &entry_idx );
	if( entry_idx ) return entry_idx - 1; 
	if( interns->len >= interns->entry_cap ){ InternGrowEntries( interns ); }
	if( interns->len >= interns->slot_half_cap ){
		InternGrowSlots( interns );
		slot_idx = InternProbe( interns, key, &entry_idx );
	}
	entry_idx = interns->len++;
	interns->slots[ slot_idx ] = entry_idx + 1; /* Must be non-zero */
	Offset offset = InternPush( interns, key->src, key->len );
	interns->entries[ entry_idx ] = ( Intern ){
		offset, key->hash, key->len, key->tag
	};
	return entry_idx;
}

InternId InternPutId( Interns* interns, u8* src, u32 len, u32 hash ){
	InternKey key = { HASH_ID, src, len, hash };
	return InternPutInternal( interns, &key );
}

InternId InternPutStr( Interns* interns, u8* src, u32 len, u32 hash ){
	InternKey key = { HASH_STR, src, len, hash };
	return InternPutInternal( interns, &key );
}

InternId InternPut( Interns* interns, u8* src ){
	u32 len = 0;
	u32 hash = HashStart( HASH_ID );
	for( ; src[ len ]; len++ ){ hash = HashU8( hash, src[ len ] ); }
	hash = HashEnd( hash );
	InternKey key = { HASH_ID, src, len, hash };
	return InternPutInternal( interns, &key );
}

u8* InternGet( Interns* interns, InternId id ){
	Intern* entry = &interns->entries[ id ];
	return AobGet( &interns->aob, entry->offset );
}

void InternFree( Interns* interns ){
	AobFree( &interns->aob );
	MemFree( interns->slots );
	MemFree( interns->entries );
}