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
	Chunks chunks;
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
	LexInit( app, &app->lexer, src_idx );
	ConstInit( &app->consts );
	LocalInit( &app->locals );
	InstInit( &app->insts );
	ChunkInit( &app->chunks );
	CompilerInit( app, &app->compiler );
	VmInit( app, &app->vm );
}

static void AppFree( App* app ){
	ChunkFree( &app->chunks );
	InstFree( &app->insts );
	LocalFree( &app->locals );
	ConstFree( &app->consts );
	InternFree( &app->interns );
	LogFree( &app->logs );
	SrcFree( &app->srcs );
}

// static void AppTestVm( App* app ){
// 	ChunkIdx chunk_idx = ChunkPush( &app->chunks );
// 	Chunk* chunk = ChunkGet( &app->chunks, chunk_idx );
// 	chunk->start = app->insts.len;

// 	ConstIdx c0 = ConstPutNum( &app->consts, 0 );
// 	ConstIdx c111 = ConstPutNum( &app->consts, 111 );
// 	ConstIdx c222 = ConstPutNum( &app->consts, 222 );

// 	InstABX( &app->insts, OP_LOADC, 0, c0 );
// 	InstABX( &app->insts, OP_JZ, 0, 4 );
// 	InstABX( &app->insts, OP_LOADC, 1, c111 );
// 	InstABX( &app->insts, OP_JMP, 0, 5 );
// 	InstABX( &app->insts, OP_LOADC, 1, c222 );
// 	InstABC( &app->insts, OP_HALT, 1, 0, 0 );

// 	chunk->len = app->insts.len - chunk->start;
// 	chunk->nregs = 2;
// 	InstDump( &app->insts );
// }

static void AppRun( App* app ){
	ChunkIdx entry = CompilerRun( &app->compiler );
	if( LogDump( &app->logs ) ) return;
	Value* value = VmRun( &app->vm, entry );
	if( value->type == VALUE_NULL ) return;
	VmPrintValue( &app->vm, value );
}

x32 main( x32 nargs, x8** args ){
	if( nargs == 1 ) Halt( ERR_NOFILE );
	App app = { 0 };
	AppInit( &app, ( u8* )args[ 1 ] );
	AppRun( &app );
	AppFree( &app );
	return 0;
}
#undef IMPL