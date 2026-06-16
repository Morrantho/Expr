#include "app.h"

static void AppFree( App* app ){
	InstFree( &app->insts );
	FnSymFree( &app->fn_syms );
	SymFree( &app->syms );
	FuncFree( &app->funcs );
	ConstFree( &app->consts );
	InternFree( &app->interns );
	LogFree( &app->logs );
	SrcFree( &app->srcs );
}

// static void AppReset( App* app, u8* text ){
// 	LogReset( &app->logs );
// 	LexReset( &app->lexer, text );
// 	InstReset( &app->insts );
// 	CompilerReset( &app->compiler );
// 	VmReset( &app->vm );
// }

/* Disabled indefinitely. */
// static void AppRepl( App* app ){
// 	u8* text = app->lexer.text; /* Lexer increments this, so copy it for resets. */
// 	for( ;; ){
// 		printf( "> " );
// 		if( !fgets( ( x8* )text, SRC_REPL_CAP, stdin ) ) return;
// 		AppReset( app, text );
// 		FuncId main = Compile( &app->compiler );
// 		if( LogDump( &app->logs ) ) continue;
// 		Value* value = VmRun( &app->vm, main );
// 		if( !value->type ) continue;
// 		VmPrintValue( &app->vm, value );
// 	}
// }

static void AppRun( App* app ){
	// if( !app->nargs ){ AppRepl( app ); return; }
	FuncId main = Compile( &app->compiler );
	if( LogDump( &app->logs ) ) return;
	InstDump( &app->insts );
	Value* value = VmRun( &app->vm, main );
	VmPrintValue( &app->vm, value );
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
	FuncInit( &app->funcs );
	SymInit( &app->syms );
	FnSymInit( &app->fn_syms );
	InstInit( &app->insts );
	CompilerInit( &app->compiler, &app->logs, &app->lexer, &app->interns, &app->consts, &app->funcs, &app->syms, &app->fn_syms, &app->insts );
	VmInit( &app->vm, &app->interns, &app->consts, &app->funcs, &app->insts );
}

x32 main( x32 nargs, x8** args ){
	if( nargs == 1 ){ Throw( ERR_BADFILE, "missing input file" ); return 1; }
	App app;
	AppInit( &app, ( u32 )nargs, ( u8** )args );
	AppRun( &app );
	AppFree( &app );
	return 0;
}