#ifndef COMPILER_H
#define COMPILER_H

#include "../lexer/lexer.h"
#include "../parsing/prec.h"
#include "../parsing/assoc.h"
#include "../parsing/deno.h"
#include "../base/const/const.h"
#include "expr.h"
#include "opcode.h"
#include "func.h"
#include "sym.h"
#include "fnsym.h"
#include "inst.h"

typedef struct Compiler {
	Logs* logs;
	Lexer* lexer;
	Interns* interns;
	Consts* consts;
	Insts* insts;
	Insts* scratch_ptr;
	Insts scratch; /* for templates */
	Funcs* funcs;
	Syms* syms;
	FnSyms* fn_syms;
	u32 reg; /* trivial register counter / allocator */
	u8 returned; /* To determine is user code has a return. */
} Compiler;

void CompilerInit( Compiler* compiler, Logs* logs, Lexer* lexer, Interns* interns, Consts* consts, Funcs* funcs, Syms* syms, FnSyms* fn_syms, Insts* insts );
void CompilerReset( Compiler* compiler );
FuncId Compile( Compiler* compiler );
void CompilerFree( Compiler* compiler );

#endif