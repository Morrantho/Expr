#ifndef COMPILER_H
#define COMPILER_H

#include "../lexer/lexer.h"
#include "../parsing/prec.h"
#include "../parsing/assoc.h"
#include "../parsing/deno.h"
#include "../base/const/const.h"

typedef u32 Reg;

typedef struct Inst {
	u8 op;
	u8 a, b, c;
} Inst;

typedef enum ExprType {
	EXPR_NONE, /* for nops */
	EXPR_ERR,
	EXPR_NUM,
	EXPR_STR,
	EXPR_REF,
	EXPR_UNKNOWN,
	EXPR_COUNT
} ExprType;

typedef struct Expr { /* 8 bytes max. If we need more metadata, use u8s for these. */
	ExprType type;
	Reg reg;
} Expr;

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