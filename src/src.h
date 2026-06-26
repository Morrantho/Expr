#ifdef TYPES
typedef u32 SrcIdx, SrcOffset;

typedef struct Src {
	SrcOffset path, text;
} Src;

typedef struct Srcs {
	Aob aob;
	Src* data;
	u32 len;
	u32 cap;
} Srcs;

typedef struct SrcSpan {
	SrcIdx src;
	SrcOffset start, end;
} SrcSpan;

typedef struct SrcPos {
	SrcIdx src;
	SrcOffset off;
	u32 ln, col;
} SrcPos;
#endif

#ifdef IMPL
void SrcInit( Srcs* srcs ){
	AobInit( &srcs->aob, SRC_AOB_CAP );
	srcs->data = MemAlloc( sizeof( Src ), SRC_VEC_CAP );
	srcs->len = 0;
	srcs->cap = SRC_VEC_CAP;
}

void SrcFree( Srcs* srcs ){
	MemFree( srcs->data );
	AobFree( &srcs->aob );
}

static FILE* SrcOpen( u8* path, u32* out_len ){
	FILE* file = fopen( ( x8* )path, "rb" );
	if( !file ){ Halt( ERR_BADFILE, path ); }
	fseek( file, 0, SEEK_END );
	long len = ftell( file );
	if( len < 0 ){ fclose( file ); Halt( ERR_FTELL, path ); }
	fseek( file, 0, SEEK_SET );
	*out_len = len;
	return file;
}

static Offset SrcReadPath( Srcs* srcs, u8* path ){
	u32 len = ( u32 )strlen( ( x8* )path );
	Offset offset = AobPush( &srcs->aob, len + 1 );
	u8* dst = AobGet( &srcs->aob, offset );
	memcpy( dst, path, len );
	dst[ len ] = '\0';
	return offset;
}

static Offset SrcReadText( Srcs* srcs, u8* path ){
	u32 len = 0;
	FILE* file = SrcOpen( path, &len );
	Offset offset = AobPush( &srcs->aob, len + 1 );
	u8* dst = AobGet( &srcs->aob, offset );
	size_t read = fread( dst, 1, len, file );
	if( read != len ){ fclose( file ); Halt( ERR_FREAD, path ); }
	dst[ len ] = '\0';
	fclose( file );
	return offset;
}

static void SrcGrow( Srcs* srcs ){
	srcs->cap <<= 1;
	srcs->data = MemRealloc( srcs->data, sizeof( Src ), srcs->cap );
}

static SrcIdx SrcPush( Srcs* srcs ){
	if( srcs->len >= srcs->cap ) SrcGrow( srcs );
	return srcs->len++;
}

Src* SrcGet( Srcs* srcs, SrcIdx idx ){
	return &srcs->data[ idx ];
}

SrcIdx SrcLoad( Srcs* srcs, u8* path ){
	SrcIdx idx = SrcPush( srcs );
	Src* src = SrcGet( srcs, idx );
	src->path = SrcReadPath( srcs, path );
	src->text = SrcReadText( srcs, path );
	return idx;
}

u8* SrcGetText( Srcs* srcs, Idx src_id ){
	Src* src = SrcGet( srcs, src_id );
	return AobGet( &srcs->aob, src->text );
}

u8* SrcGetPath( Srcs* srcs, Idx src_id ){
	Src* src = SrcGet( srcs, src_id );
	return AobGet( &srcs->aob, src->path );
}
#endif