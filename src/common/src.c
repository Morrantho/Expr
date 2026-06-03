#include "src.h"

void SrcInit( SrcList* list, u32 cap ){
	list->data = MemAlloc( sizeof( Src ), cap );
	list->cap = cap;
	list->len = 0;
}

static void SrcGrow( SrcList* list ){
	list->cap <<= 1;
	list->data = MemRealloc( list->data, sizeof( Src ), list->cap );
}

static SrcId SrcPush( SrcList* list ){
	if( list->len >= list->cap ) SrcGrow( list );
	SrcId id = list->len++;
	list->data[ id ] = ( Src ){ 0 };
	return id;
}

Src* SrcGet( SrcList* list, SrcId id ){
	return &list->data[ id ];
}

// static x8* SrcRead( x8* path, u32* out_len ){
// 	FILE* file = fopen( path, "rb" );
// 	if( !file ) Throw( ERR_BADFILE, path );
// 	fseek( file, 0, SEEK_END );
// 	long len = ftell( file );
// 	if( len < 0 ) Throw( ERR_FTELL, path );
// 	fseek( file, 0, SEEK_SET );
// 	x8* text = MemAlloc( 1, len + 1 );
// 	size_t bytes = fread( text, 1, len, file );
// 	fclose( file );
// 	if( bytes != len ) Throw( ERR_FREAD, path );
// 	text[ len ] = '\0';
// 	*out_len = len;
// 	return text;
// }

static FILE* SrcOpen( x8* path, u32* out_len ){
	if( !path ){ *out_len = SRC_REPL_MAX; return stdin; }
	FILE* file = fopen( path, "rb" );
	if( !file ) Throw( ERR_BADFILE, path );
	fseek( file, 0, SEEK_END );
	long len = ftell( file );
	if( len < 0 ){ fclose( file ); Throw( ERR_FTELL, path ); }
	fseek( file, 0, SEEK_SET );
	*out_len = len;
	return file;
}

static x8* SrcRead( x8* path, u32* out_len ){
	FILE* file = SrcOpen( path, out_len );
	x8* text = MemAlloc( 1, *out_len + 1 );
	if( !path ) return text; /* file == stdin works too */
	size_t bytes = fread( text, 1, *out_len, file );
	fclose( file );
	if( bytes != *out_len ) Throw( ERR_FREAD, path );
	text[ *out_len ] = '\0';
	return text;
}

static x8* SrcPath( x8* path ){
	if( !path ) path = "stdin";
	size_t len = strlen( path );
	x8* dst = MemAlloc( sizeof( x8 ), len + 1 );
	memcpy( dst, path, len + 1 ); /* Copy \0 */
	return dst;
}

Src* SrcLoad( SrcList* list, x8* path ){
	Src* src = SrcGet( list, SrcPush( list ) );
	src->path = SrcPath( path );
	src->text = SrcRead( path, &src->len );
	return src;
}

void SrcFree( SrcList* list ){
	for( u32 i = 0; i < list->len; ++i ){
		MemFree( list->data[ i ].path );
		MemFree( list->data[ i ].text );
		list->data[ i ] = ( Src ){ 0 };
	}
	MemFree( list->data );
	list->data = 0;
	list->cap = list->len = 0;
}