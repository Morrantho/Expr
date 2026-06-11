#ifndef COMPILER_H
#define COMPILER_H

#include "../lexer/lexer.h"
#include "../parsing/prec.h"
#include "../parsing/assoc.h"
#include "../parsing/deno.h"
#include "../base/const/const.h"
#include "expr.h"
#include "opcode.h"

typedef struct Inst {
	u8 op;
	u8 a, b, c;
} Inst;

typedef struct Compiler {
	Logs* logs;
	Lexer* lexer;
	Consts* consts;

	Inst* code;
	u32 len;
	u32 cap;

	u32 reg; /* trivial register counter / allocator */
} Compiler;

void CompilerInit( Compiler* compiler, Logs* logs, Lexer* lexer, Consts* consts );
void CompilerReset( Compiler* compiler );
Expr Compile( Compiler* compiler );
void CompilerFree( Compiler* compiler );

#endif