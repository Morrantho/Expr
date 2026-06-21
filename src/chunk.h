#ifdef TYPES
typedef u16 ChunkIdx;
#define CHUNK_NONE UINT16_MAX

typedef struct Chunk { /* instruction span */
	InstIdx start;
	InstIdx len;
	u32 nregs;
} Chunk;

typedef struct Chunks {
	Chunk* data;
	u32 len, cap;
} Chunks;
#endif

#ifdef IMPL
void ChunkInit( Chunks* chunks ){
	chunks->data = MemAlloc( sizeof( chunks->data[ 0 ] ), CHUNK_VEC_CAP );
	chunks->len = 0;
	chunks->cap = CHUNK_VEC_CAP;
}

void ChunkFree( Chunks* chunks ){
	MemFree( chunks->data );
	chunks->data = 0;
	chunks->len = chunks->cap = 0;
}

static void ChunkGrow( Chunks* chunks ){
	chunks->cap <<= 1;
	chunks->data = MemRealloc( chunks->data, sizeof( chunks->data[ 0 ] ), chunks->cap );
}

ChunkIdx ChunkPush( Chunks* chunks ){
	if( chunks->len >= chunks->cap ) ChunkGrow( chunks );
	return chunks->len++;
}

Chunk* ChunkGet( Chunks* chunks, ChunkIdx idx ){
	return &chunks->data[ idx ];
}
#endif