#ifndef APP_H
#define APP_H

#include "../common/src.h"
#include "../lexer/lexer.h"

typedef struct App {
	u32 nargs;
	x8** args;
	SrcList sources;
	Lexer lexer;
} App;

x32 main( x32 nargs, x8** args );

#endif