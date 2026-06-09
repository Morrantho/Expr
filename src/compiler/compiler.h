#ifndef COMPILER_H
#define COMPILER_H

#include "../lexer/lexer.h"
#include "../parser/prec.h"
#include "../parser/assoc.h"
#include "../parser/deno.h"

typedef u32 Reg;

typedef struct Inst {
	Reg op;
	Reg a, b, c;
} Inst;

typedef struct Compiler {
	Logs* logs;
	Lexer* lexer;

	Inst* code;
	u32 len;
	u32 cap;

	Reg reg;
} Compiler;

void CompilerInit( Compiler* compiler, Logs* logs, Lexer* lexer );
void CompilerReset( Compiler* compiler );
Reg Compile( Compiler* compiler );
void CompilerFree( Compiler* compiler );

#endif