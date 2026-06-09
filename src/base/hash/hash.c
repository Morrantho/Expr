#include "hash.h"

u32 HashU8( u32 hash, u8 byte ){
	hash ^= byte;
	hash *= HASH_PRIME;
	return hash;
}

u32 HashStart( HashTag tag ){
	return HashU8( HASH_OFFSET, ( u8 )tag );
}
/* We cant have hashes equal 0, because 0 means "empty slot" in our hashmaps. */
u32 HashEnd( u32 hash ){
	return hash + !hash;
}