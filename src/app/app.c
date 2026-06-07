#include "app.h"

static void AppFree( App* app ){
	InternFree( &app->interns );
	LogFree( &app->logs );
	SrcFree( &app->srcs );
}

static void AppReset( App* app, u8* text ){
	LogReset( &app->logs );
	LexReset( &app->lexer, text );
}

static void AppRepl( App* app ){
	u8* text = app->lexer.text; /* Lexer increments this, so copy it for resets. */
	for( ;; ){
		printf( "> " );
		if( !fgets( ( x8* )text, SRC_REPL_CAP, stdin ) ) return;
		AppReset( app, text );
		/* We'll get rid of this test soon. */
		for( ;; ){
			Lex( &app->lexer );
			if( app->lexer.tk.type == TK_EOS ) break;
			printf( "%u\n", app->lexer.tk.intern );
			//printf( "%s\n", TkGetType( &app->lexer.tk ) );
		}

		if( LogDump( &app->logs ) ) continue;
		// Compile( app );
		// Run( app );
	}
}

static void AppRun( App* app ){
	if( !app->nargs ){ AppRepl( app ); return; }
	// Compile( app );
	if( LogDump( &app->logs ) ) return;
	// Run( app );
}

static void AppInit( App* app, u32 nargs, u8** args ){
	app->nargs = nargs - 1;
	app->args = args;
	SrcInit( &app->srcs );
	SrcId src_id = SrcLoad( &app->srcs, app->args[ 1 ] );
	u8* text = SrcGetText( &app->srcs, src_id );
	LogInit( &app->logs, &app->srcs );
	InternInit( &app->interns );
	LexInit( &app->lexer, &app->logs, &app->interns, src_id, text );
}

x32 main( x32 nargs, x8** args ){
	App app;
	AppInit( &app, ( u32 )nargs, ( u8** )args );
	AppRun( &app );
	AppFree( &app );
	return 0;
}