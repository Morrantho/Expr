#ifdef TYPES
typedef struct Aob {
	u8* data;
	u32 len, cap;
} Aob;
#endif

#ifdef IMPL
void AobInit( Aob* aob, u32 cap ){
	aob->data = MemAlloc( 1, cap );
	aob->len = 0;
	aob->cap = cap;
}

void AobFree( Aob* aob ){
	MemFree( aob->data );
	aob->data = 0;
}

void AobReset( Aob* aob ){
	aob->len = 0;
}

static void AobGrow( Aob* aob, u32 total ){
	while( total > aob->cap ){ aob->cap <<= 1; }
	aob->data = MemRealloc( aob->data, 1, aob->cap );
}

Offset AobPush( Aob* aob, u32 count ){
	u32 len = aob->len;
	u32 total = len + count;
	if( total > aob->cap ){ AobGrow( aob, total ); }
	aob->len = total;
	return len;
}

u8* AobGet( Aob* aob, Offset offset ){
	return aob->data + offset;
}
#endif