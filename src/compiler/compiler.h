#ifndef COMPILER_H
#define COMPILER_H

#include "../lexer/lexer.h"
#include "../parser/prec.h"
#include "../parser/assoc.h"
#include "../parser/deno.h"
#include "../base/const/const.h"

typedef u32 Reg;

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
Reg Compile( Compiler* compiler );
void CompilerFree( Compiler* compiler );

#endif