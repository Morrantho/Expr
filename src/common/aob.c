#include "aob.h"
/* We only use these as an array of bytes, so alignment is always 1. */
/* If you need a generic vector, you should make one. This isnt a */
/* typical arena allocator, we dont need it. */
void AobInit( Aob* aob, u32 cap ){
	aob->bytes = MemAlloc( 1, cap );
	aob->len = 0;
	aob->cap = cap;
}

void AobReset( Aob* aob ){
	aob->len = 0;
}

static void AobGrow( Aob* aob, u32 total ){
	while( total > aob->cap ) aob->cap <<= 1;
	aob->bytes = MemRealloc( aob->bytes, 1, aob->cap );
}

Offset AobPush( Aob* aob, u32 len ){
	Offset off = aob->len;
	u32 total = off + len;
	if( total > aob->cap ) AobGrow( aob, total );
	aob->len = total;
	return off;
}

x8* AobGet( Aob* aob, Offset offset ){
	return aob->bytes + offset;
}

void AobFree( Aob* aob ){
	MemFree( aob->bytes );
	*aob = ( Aob ){ 0 };
}