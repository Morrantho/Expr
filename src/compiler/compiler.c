#include "compiler.h"

void CompilerInit( Compiler* compiler, Logs* logs, Lexer* lexer ){
	compiler->logs = logs;
	compiler->lexer = lexer;
	compiler->code = MemAlloc( sizeof( Inst ), CMP_CODE_CAP );
	compiler->len = 0;
	compiler->cap = CMP_CODE_CAP;
	compiler->reg = 0;
}

void CompilerReset( Compiler* compiler ){
	compiler->len = 0;
	compiler->reg = 0;
}

static Reg RegAlloc( Compiler* compiler, LogPos* pos ){
	if( compiler->reg >= CMP_REG_CAP ){
		Log( compiler->logs, pos, CMP_REGALLOC );
		return CMP_REG_CAP;
	}
	return compiler->reg++;
}

static Reg CompileBadPrefix( Compiler* compiler, Deno deno, Tk* tk ){
	u8 *deno_name = DenoGetName( deno );
	u8 *tk_name = TkGetName( tk->type );
	Log( compiler->logs, &tk->pos, PARSE_BADPRE, deno_name, tk_name );
	return CMP_REG_CAP;
}

static Reg CompileNum( Compiler* compiler, Tk* tk ){
	Reg dst = RegAlloc( compiler, &tk->pos );
	/* Just stubbing in will eventually exist: */
	/* ConstId cid = ConstPut( compiler->consts, tk->num ) */
	/* EmitABC( compiler, OP_LOADC, dst, cid, 0 ); */
	return dst;
}

static Reg CompilePrefix( Compiler* compiler ){
	Lexer* lexer = compiler->lexer;
	Tk tk = lexer->tk; /* copy */
	Deno deno = DenoGet( PARSEPOS_PRE, tk.type );
	Lex( lexer );
	switch( deno ){ /* We'll macro away dispatch same as lexer + vm later. */
		default: return CompileBadPrefix( compiler, deno, &tk );
		case DENO_NUM: return CompileNum( compiler, &tk );
	}
}

// static Reg CompileExpr( Compiler* compiler, Prec min ){
static Reg CompileExpr( Compiler* compiler ){
	Reg reg = CompilePrefix( compiler );
	// reg = CompilePostfix( compiler, reg );
	// reg = CompileInfix( compiler, reg, min );
	return reg;
}

Reg Compile( Compiler* compiler ){
	Lex( compiler->lexer );
	Reg dst = CompileExpr( compiler );
	// Emit( );
	return dst;
}

void CompilerFree( Compiler* compiler ){
	MemFree( compiler->code );
}