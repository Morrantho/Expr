#define BASE
#include "includes.h"
#undef BASE

#define TYPES
#include "includes.h"
typedef struct App {
	Srcs srcs;
	Logs logs;
	Interns interns;
	Consts consts;
	Locals locals;
	Insts insts;
	Patches ifs, loops;
	Fns fns;
	Lexer lexer;
	Compiler compiler;
	Vm vm;
} App;
#undef TYPES

#define IMPL
#include "includes.h"
static void AppInit( App* app, u8* path ){
	SrcInit( &app->srcs );
	SrcIdx src_idx = SrcLoad( &app->srcs, path );
	LogInit( &app->logs, &app->srcs );
	InternInit( &app->interns );
	LexInit( &app->lexer, app, src_idx );
	ConstInit( &app->consts );
	LocalInit( &app->locals );
	InstInit( &app->insts );
	PatchInit( &app->ifs, &app->insts );
	PatchInit( &app->loops, &app->insts );
	FnInit( &app->fns );
	NativeInit( app );
	CompilerInit( &app->compiler, app );
	VmInit( &app->vm, app );
}

static void AppFree( App* app ){
	FnFree( &app->fns );
	PatchFree( &app->loops );
	PatchFree( &app->ifs );
	InstFree( &app->insts );
	LocalFree( &app->locals );
	ConstFree( &app->consts );
	InternFree( &app->interns );
	LogFree( &app->logs );
	SrcFree( &app->srcs );
}

static void AppRun( App* app ){
	InstIdx entry = CompilerRun( &app->compiler );
	if( LogDump( &app->logs ) ){ return; }
	VmRun( &app->vm, entry );
}

x32 main( x32 nargs, x8** args ){
	if( nargs == 1 ){ Halt( ERR_NOFILE ); }
	App app = { 0 };
	AppInit( &app, ( u8* )args[ 1 ] );
	AppRun( &app );
	AppFree( &app );
	return 0;
}
#undef IMPL