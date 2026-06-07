#include "intern.h"

void InternInit( Interns* table ){
	AobInit( &table->aob, INTERN_AOB_CAP );
	table->entries = MemCalloc( sizeof( Intern ), INTERN_CAP ); /* 0 for hashes */
	table->cap = INTERN_CAP;
	table->half_cap = INTERN_CAP >> 1;
	table->len = 0;
}

static void InternReput( Interns* table, Intern* old_entry ){
	u32 mask = table->cap - 1;
	u32 idx = old_entry->hash & mask;
	while( table->entries[ idx ].hash ) /* probe until empty found */
		idx = ( idx + 1 ) & mask;
	table->entries[ idx ] = *old_entry;
	table->len++;
}

static void InternGrow( Interns* table ){
	Intern* old_entries = table->entries;
	u32 old_cap = table->cap;
	table->cap <<= 1;
	table->half_cap = old_cap; /* table->cap >> 1; */
	table->entries = MemCalloc( sizeof( Intern ), table->cap );
	table->len = 0;
	for( u32 i = 0; i < old_cap; i++ ){
		Intern* old_entry = &old_entries[ i ];
		if( !old_entry->hash ) continue;
		InternReput( table, old_entry );
	}
	MemFree( old_entries );
}

static Intern* InternProbe( Interns* table, HashTag tag, u8* src, u32 len, u32 hash ){
	u32 mask = table->cap - 1;
	u32 idx = hash & mask;
	Intern* entries = table->entries;
	Aob* aob = &table->aob;
	for( ;; idx = ( idx + 1 ) & mask ){
		Intern* entry = &entries[ idx ];
		if( !entry->hash ) return entry; /* empty slot */
		if( entry->len != len || entry->hash != hash ) continue; /* no match */
		if( entry->tag != tag ) continue; /* No need to touch AOB. */
		u8* dst = AobGet( aob, entry->offset );
		if( memcmp( src, dst, len ) == 0 ) return entry; /* full match */
	}
}

static Offset InternPush( Interns* table, u8* src, u32 len ){
	Aob* aob = &table->aob;
	Offset off = AobPush( aob, len + 1 );
	u8* dst = AobGet( aob, off );
	memcpy( dst, src, len );
	dst[ len ] = '\0';
	return off;
}

Offset InternPut( Interns* table, HashTag tag, u8* src, u32 len, u32 hash ){
	Intern* entry = InternProbe( table, tag, src, len, hash );
	if( entry->hash ) return entry->offset;
	if( table->len >= table->half_cap ){
		InternGrow( table );
		entry = InternProbe( table, tag, src, len, hash );
	}
	entry->offset = InternPush( table, src, len );
	entry->hash = hash;
	entry->len = len;
	entry->tag = tag;
	table->len++;
	return entry->offset;
}

Offset InternPutId( Interns* table, u8* src, u32 len, u32 hash ){
	return InternPut( table, HASH_ID, src, len, hash );
}

Offset InternPutStr( Interns* table, u8* src, u32 len, u32 hash ){
	return InternPut( table, HASH_STR, src, len, hash );
}

u8* InternGetRaw( Interns* table, Offset off ){
	return AobGet( &table->aob, off );
}

void InternFree( Interns* table ){
	AobFree( &table->aob );
	MemFree( table->entries );
}