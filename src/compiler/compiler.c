#include "compiler.h"

static Reg CompileExpr( Compiler* compiler, Prec min );

void CompilerInit( Compiler* compiler, Logs* logs, Lexer* lexer, Consts* consts ){
	compiler->logs = logs;
	compiler->lexer = lexer;
	compiler->consts = consts;
	compiler->code = MemAlloc( sizeof( Inst ), CMP_CODE_CAP );
	compiler->len = 0;
	compiler->cap = CMP_CODE_CAP;
	compiler->reg = 0;
}

void CompilerReset( Compiler* compiler ){
	compiler->len = 0;
	compiler->reg = 0;
}
/* Temporary until we deal with blocks, scopes, functions, etc. */
static Reg RegAlloc( Compiler* compiler ){
	if( compiler->reg >= CMP_REG_CAP ){
		Throw( CMP_REG_CAP );
		return 0;
	}
	return compiler->reg++;
}

static void CompilerMatch( Compiler* compiler, TkType expected ){
	Lexer* lexer = compiler->lexer;
	Tk* tk = &lexer->tk;
	if( tk->type != expected ){ /* Continue to parse, so we log all their issues. */
		Log( compiler->logs, &tk->pos, PARSE_EXPECT, expected, tk->type );
	}
	Lex( lexer );
}

static Reg CompileBadPrefix( Compiler* compiler, Deno deno, Tk* tk ){
	u8 *deno_name = DenoGetName( deno );
	u8 *tk_name = TkGetName( tk->type );
	Log( compiler->logs, &tk->pos, PARSE_BADPRE, deno_name, tk_name );
	return CMP_REG_CAP;
}

static Reg CompileGroup( Compiler* compiler ){
	Reg src = CompileExpr( compiler, PREC_NONE );
	CompilerMatch( compiler, TK_RP );
	return src;
}

static Reg CompileNum( Compiler* compiler, Tk* tk ){
	Reg dst = RegAlloc( compiler );
	ConstId cid = ConstPutNum( compiler->consts, tk->num );
	printf( "const num: %d\n", cid ); /* no emission yet, just log consts. */
	/* EmitABC( compiler, OP_LOADC, dst, cid, 0 ); */
	return dst;
}

static Reg CompileUnary( Compiler* compiler, Tk* tk ){
	Reg src = CompileExpr( compiler, PREC_UNARY );
	Reg dst = RegAlloc( compiler );
	printf( "CompileUnary: tk: %d src: %d dst: %d\n", tk->type, src, dst );
	/* Emit( &compiler->code, OpGet( POS_PRE, tk->type ), dst, src,  ); */
	return dst;
}

static Reg CompilePrefix( Compiler* compiler ){
	Lexer* lexer = compiler->lexer;
	Tk tk = lexer->tk; /* copy */
	Deno deno = DenoGet( PARSEPOS_PRE, tk.type );
	Lex( lexer );
	switch( deno ){
		default: return CompileBadPrefix( compiler, deno, &tk );
		case DENO_NOP: return CMP_REG_CAP;
		case DENO_NOPPRE: return CompileExpr( compiler, PREC_UNARY );
		case DENO_GRP: return CompileGroup( compiler );
		case DENO_PRE: return CompileUnary( compiler, &tk );
		case DENO_NUM: return CompileNum( compiler, &tk );
	}
}

static Reg CompilePostUnary( Compiler* compiler, Lexer* lexer, Reg src, Tk* tk ){
	Lex( lexer );
	Reg dst = RegAlloc( compiler );
	printf( "CompilePostUnary: src: %d dst: %d tk: %d\n", src, dst, tk->type );
	/* Emit( &compiler->code, OpGet( POS_POST, tk->type ), dst, src,  ); */
	return dst;
}

static Reg CompilePostfix( Compiler* compiler, Reg src ){
	Lexer* lexer = compiler->lexer;
	for( ;; ){ /* member access + calls can chain. not yet implemented. */
		Tk tk = lexer->tk; /* copy required */
		Deno deno = DenoGet( PARSEPOS_POST, tk.type );
		switch( deno ){
			default: return src;
			case DENO_POST: src = CompilePostUnary( compiler, lexer, src, &tk ); break;
		}
	}
}

static Reg CompileBinary( Compiler* compiler, Lexer* lexer, Reg lhs, Prec prec, Tk* tk ){
	Lex( lexer );
	Reg rhs = CompileExpr( compiler, prec );
	Reg dst = RegAlloc( compiler );
	printf( "CompileBinary: lhs: %d tk: %d rhs: %d\n", lhs, tk->type, rhs );
	return dst;
}

static Reg CompileInfix( Compiler* compiler, Reg lhs, Prec min ){
	Lexer* lexer = compiler->lexer;
	Tk tk = lexer->tk;
	while( DenoGet( PARSEPOS_INF, tk.type ) == DENO_INF ){
		Prec prec = PrecGet( tk.type );
		Assoc assoc = AssocGet( tk.type );
		if( prec - assoc <= min ) break;
		lhs = CompileBinary( compiler, lexer, lhs, prec, &tk );
		tk = lexer->tk;
	}
	return lhs;
}

static Reg CompileExpr( Compiler* compiler, Prec min ){
	Reg reg = CompilePrefix( compiler );
	reg = CompilePostfix( compiler, reg );
	reg = CompileInfix( compiler, reg, min );
	return reg;
}

Reg Compile( Compiler* compiler ){
	Lex( compiler->lexer );
	Reg dst = CompileExpr( compiler, PREC_NONE );
	// Emit( &compiler->code, OP_HALT, 0, 0, 0 );
	return dst;
}

void CompilerFree( Compiler* compiler ){
	MemFree( compiler->code );
}