#ifndef APP_H
#define APP_H

#include "../base/src/src.h"
#include "../base/diag/log.h"
#include "../lexer/lexer.h"
#include "../compiler/compiler.h"

typedef struct App {
	u32 nargs;
	u8** args;
	Logs logs;
	Srcs srcs;
	Interns interns;
	Lexer lexer;
	Compiler compiler;
	// Vm vm;
} App;

x32 main( x32 nargs, x8** args );

#endif