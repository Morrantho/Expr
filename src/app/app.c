#include "app.h"

static void AppFree( App* app ){
	// VmFree( &app->vm );
	// CompilerFree( &app->compiler );
	InstFree( &app->insts );
	ConstFree( &app->consts );
	InternFree( &app->interns );
	LogFree( &app->logs );
	SrcFree( &app->srcs );
}

static void AppReset( App* app, u8* text ){
	LogReset( &app->logs );
	LexReset( &app->lexer, text );
	InstReset( &app->insts );
	CompilerReset( &app->compiler );
	VmReset( &app->vm );
}

static void AppRepl( App* app ){
	u8* text = app->lexer.text; /* Lexer increments this, so copy it for resets. */
	for( ;; ){
		printf( "> " );
		if( !fgets( ( x8* )text, SRC_REPL_CAP, stdin ) ) return;
		AppReset( app, text );
		Compile( &app->compiler );
		if( LogDump( &app->logs ) ) continue;
		Value* value = VmRun( &app->vm );
		if( !value->type ) continue;
		VmPrintValue( &app->vm, value );
	}
}

static void AppRun( App* app ){
	if( !app->nargs ){ AppRepl( app ); return; }
	Compile( &app->compiler );
	if( LogDump( &app->logs ) ) return;
	VmRun( &app->vm );
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
	ConstInit( &app->consts );
	InstInit( &app->insts );
	CompilerInit( &app->compiler, &app->logs, &app->lexer, &app->consts, &app->insts );
	VmInit( &app->vm, &app->interns, &app->consts, &app->insts );
}

x32 main( x32 nargs, x8** args ){
	App app;
	AppInit( &app, ( u32 )nargs, ( u8** )args );
	AppRun( &app );
	AppFree( &app );
	return 0;
}