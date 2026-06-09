#include "mem.h"

void* MemAlloc( u32 szof, u32 count ){
	u32 total = szof * count;
	void* block = malloc( total );
	if( !block ) Throw( ERR_OOM, total );
	return block;
}

void* MemCalloc( u32 szof, u32 count ){
	u32 total = szof * count;
	void* block = calloc( count, szof );
	if( !block ) Throw( ERR_OOM, total );
	return block;
}

void* MemRealloc( void* block, u32 szof, u32 count ){
	u32 total = szof * count;
	void* new_block = realloc( block, total );
	if( !block ) Throw( ERR_OOM, total );
	return new_block;
}

void MemFree( void* block ){
	if( !block ) Throw( ERR_FREE );
	free( block );
}