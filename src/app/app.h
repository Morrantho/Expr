#ifndef APP_H
#define APP_H

#include "../common/src.h"
#include "../common/log.h"
#include "../lexer/lexer.h"

typedef struct App {
	u32 nargs;
	u8** args;
	SrcList sources;
	Lexer lexer;
	// Parser parser;
	// Compiler compiler;
	// Vm vm;
} App;

x32 main( x32 nargs, x8** args );

#endif