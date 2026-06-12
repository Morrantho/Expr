#ifndef COMPILER_H
#define COMPILER_H

#include "../lexer/lexer.h"
#include "../parsing/prec.h"
#include "../parsing/assoc.h"
#include "../parsing/deno.h"
#include "../base/const/const.h"
#include "expr.h"
#include "opcode.h"
#include "inst.h"

typedef struct Compiler {
	Logs* logs;
	Lexer* lexer;
	Consts* consts;
	Insts* insts;
	u32 reg; /* trivial register counter / allocator */
} Compiler;

void CompilerInit( Compiler* compiler, Logs* logs, Lexer* lexer, Consts* consts, Insts* insts );
void CompilerReset( Compiler* compiler );
Expr Compile( Compiler* compiler );
// void CompilerFree( Compiler* compiler );

#endif