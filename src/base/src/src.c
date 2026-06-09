#include "src.h"

void SrcInit( Srcs* list ){
	AobInit( &list->aob, SRC_AOB_CAP );
	list->sources = MemAlloc( sizeof( Src ), SRC_CAP );
	list->cap = SRC_CAP;
	list->len = 0;
}

static void SrcGrow( Srcs* list ){
	list->cap <<= 1;
	list->sources = MemRealloc( list->sources, sizeof( Src ), list->cap );
}

static SrcId SrcPush( Srcs* list ){
	if( list->len >= list->cap ) SrcGrow( list );
	SrcId id = list->len++;
	list->sources[ id ] = ( Src ){ 0 };
	return id;
}

Src* SrcGet( Srcs* list, SrcId src_id ){
	return &list->sources[ src_id ];
}

u8* SrcGetPath( Srcs* list, SrcId src_id ){
	Src* src = SrcGet( list, src_id );
	return AobGet( &list->aob, src->path );
}

u8* SrcGetText( Srcs* list, SrcId src_id ){
	Src* src = SrcGet( list, src_id );
	return AobGet( &list->aob, src->text );
}

static FILE* SrcOpen( u8* path, u32* out_len ){
	if( !path ){ *out_len = SRC_REPL_CAP; return stdin; }
	FILE* file = fopen( ( x8* )path, "rb" );
	if( !file ) Throw( ERR_BADFILE, path );
	fseek( file, 0, SEEK_END );
	long len = ftell( file );
	if( len < 0 ){ fclose( file ); Throw( ERR_FTELL, path ); }
	fseek( file, 0, SEEK_SET );
	*out_len = len;
	return file;
}

static Offset SrcRead( Srcs* list, u8* path, u32* out_len ){
	FILE* file = SrcOpen( path, out_len );
	Offset byte_off = AobPush( &list->aob, *out_len + 1 );
	if( !path ) return byte_off; /* file == stdin works too */
	u8* text = AobGet( &list->aob, byte_off );
	size_t bytes_read = fread( text, 1, *out_len, file );
	fclose( file );
	if( bytes_read != *out_len ) Throw( ERR_FREAD, path );
	text[ *out_len ] = '\0';
	return byte_off;
}

static Offset SrcPath( Srcs* list, u8* path ){
	if( !path ) path = ( u8*)"stdin";
	size_t len = strlen( ( x8* )path );
	Offset byte_off = AobPush( &list->aob, len + 1 );
	u8* dst = AobGet( &list->aob, byte_off );
	memcpy( dst, path, len + 1 ); /* Copy \0 */
	return byte_off;
}

SrcId SrcLoad( Srcs* list, u8* path ){
	SrcId src_id = SrcPush( list );
	Src* src = SrcGet( list, src_id );
	src->path = SrcPath( list, path );
	src->text = SrcRead( list, path, &src->len );
	return src_id;
}

void SrcFree( Srcs* list ){
	AobFree( &list->aob );
	MemFree( list->sources );
	list->sources = 0;
	list->cap = list->len = 0;
}