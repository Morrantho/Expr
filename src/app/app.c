#include "app.h"

static void AppFree( App* app ){
	SrcFree( &app->sources );
}

static void AppReset( App* app, x8* text ){
	LexReset( &app->lexer, text );
}

static void AppRepl( App* app ){
	Src* src = app->lexer.src;
	x8* text = src->text;
	for( ;; ){
		printf( "> " );
		if( !fgets( src->text, src->len, stdin ) ) return;
		AppReset( app, text );
		// Compile( app );
		// Run( app );
	}
}

static void AppRun( App* app ){
	if( !app->nargs ){ AppRepl( app ); return; }
	// Compile( app );
	// Run( app );
}

static void AppInit( App* app, u32 nargs, x8** args ){
	app->nargs = nargs - 1;
	app->args = args;
	SrcInit( &app->sources, SRC_LIST_MAX );
	Src* src = SrcLoad( &app->sources, app->args[ 1 ] );
	LexInit( &app->lexer, src );
}

x32 main( x32 nargs, x8** args ){
	App app;
	AppInit( &app, ( u32 )nargs, args );
	AppRun( &app );
	AppFree( &app );
	return 0;
}