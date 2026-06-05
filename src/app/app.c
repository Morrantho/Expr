#include "app.h"

static void AppFree( App* app ){
	LogFree( &app->logs );
	SrcFree( &app->sources );
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
			printf( "%s\n", TkGetType( &app->lexer.tk ) );
		}

		LogFlush( &app->logs );
		if( LogIsFatal( &app->logs ) ) continue;
		// Compile( app );
		// Run( app );
	}
}

static void AppRun( App* app ){
	if( !app->nargs ){ AppRepl( app ); return; }
	// Compile( app );
	LogFlush( &app->logs );
	if( LogIsFatal( &app->logs ) ) return;
	// Run( app );
}

static void AppInit( App* app, u32 nargs, u8** args ){
	app->nargs = nargs - 1;
	app->args = args;
	LogInit( &app->logs, LOG_AOB_CAP, LOG_ENTRY_CAP );
	SrcInit( &app->sources, SRC_LIST_CAP );
	Src* src = SrcLoad( &app->sources, app->args[ 1 ] );
	LexInit( &app->lexer, &app->logs, src );
}

x32 main( x32 nargs, x8** args ){
	App app;
	AppInit( &app, ( u32 )nargs, ( u8** )args );
	AppRun( &app );
	AppFree( &app );
	return 0;
}