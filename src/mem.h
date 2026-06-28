#ifdef IMPL
void* MemAlloc( u32 szof, u32 count ){
	void* block = malloc( szof * count );
	if( !block ){ Halt( ERR_OOM, szof * count ); }
	return block;
}

void* MemCalloc( u32 szof, u32 count ){
	void* block = calloc( count, szof );
	if( !block ){ Halt( ERR_OOM, szof * count ); }
	return block;
}

void* MemRealloc( void* block, u32 szof, u32 count ){
	void* new_block = realloc( block, szof * count );
	if( !new_block ){ Halt( ERR_OOM, szof * count ); }
	return new_block;
}

void MemFree( void* block ){
	if( !block ){ Halt( ERR_FREE ); }
	free( block );
}
#endif