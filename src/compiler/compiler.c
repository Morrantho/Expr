#include "compiler.h"

static Expr CompileExpr( Compiler* compiler, Prec min );

void CompilerInit( Compiler* compiler, Logs* logs, Lexer* lexer, Consts* consts, Insts* insts ){
	compiler->logs = logs;
	compiler->lexer = lexer;
	compiler->consts = consts;
	compiler->insts = insts;
	compiler->reg = 0;
}

void CompilerReset( Compiler* compiler ){
	compiler->reg = 0;
}

/* Temporary until we deal with blocks, scopes, functions, etc. */
static Reg RegAlloc( Compiler* compiler ){
	if( compiler->reg >= CMP_REG_CAP ){
		Throw( ERR_REGALLOC );
		return 0;
	}
	return ( Reg )compiler->reg++;
}

static void CompilerMatch( Compiler* compiler, TkType expected ){
	Lexer* lexer = compiler->lexer;
	Tk* tk = &lexer->tk;
	if( tk->type != expected ){ /* Continue to parse, so we log all their issues. */
		Log( compiler->logs, &tk->pos, PARSE_EXPECT, expected, tk->type );
	}
	Lex( lexer );
}

static Expr CompileBadPrefix( Compiler* compiler, Deno deno, Tk* tk ){
	u8 *deno_name = DenoGetName( deno );
	u8 *tk_name = TkGetName( tk->type );
	Log( compiler->logs, &tk->pos, PARSE_BADPRE, deno_name, tk_name );
	return ExprGen( EXPR_ERR, CMP_REG_ERR );
}

static Expr CompileGroup( Compiler* compiler ){
	Expr src = CompileExpr( compiler, PREC_NONE );
	CompilerMatch( compiler, TK_RP );
	return src;
}

static Expr CompileUnary( Compiler* compiler, Tk* tk ){
	Expr src = CompileExpr( compiler, PREC_UNARY );
	Op op = OpGetUnary( src.type, tk->type );
	Expr dst = ExprGen( op.type, RegAlloc( compiler ) );
	// printf( "CompileUnary: tk: %d src: %d dst: %d\n", tk->type, src.reg, dst.reg );
	/* Emit( &compiler->code, op.code, dst.reg, src.reg,  ); */
	return dst;
}

static Expr CompileNum( Compiler* compiler, Tk* tk ){
	Expr dst = ExprGen( EXPR_NUM, RegAlloc( compiler ) );
	ConstId cid = ConstPutNum( compiler->consts, tk->num );
	printf( "const num: %d\n", cid ); /* no emission yet, just log consts. */
	/* EmitABC( compiler, OP_LOADC, dst, cid, 0 ); */
	return dst;
}

static Expr CompileStr( Compiler* compiler, Tk* tk ){
	Expr dst = ExprGen( EXPR_STR, RegAlloc( compiler ) );
	ConstId cid = ConstPutStr( compiler->consts, tk->intern );
	printf( "const str: %d\n", cid );
	/* EmitABC( compiler, OP_LOADC, dst, cid, 0 ); */
	return dst;
}

static Expr CompileRef( Compiler* compiler, Tk* tk ){
	Expr dst = ExprGen( EXPR_REF, RegAlloc( compiler ) );
	/* Sym sym = SymbolGet( compiler->symbols, tk->intern ); */
	printf( "load ref: %d\n", tk->intern );
	/* EmitABC( compiler, OP_LOADLOCAL | OP_LOADGLOBAL | OP_LOADUPVAL, dst, sym ) */
	return dst;
}

static Expr CompilePrefix( Compiler* compiler ){
	Lexer* lexer = compiler->lexer;
	Tk tk = lexer->tk; /* copy */
	Deno deno = DenoGet( PARSEPOS_PRE, tk.type );
	Lex( lexer );
	switch( deno ){
		default: return CompileBadPrefix( compiler, deno, &tk );
		case DENO_NOPPRE: return CompileExpr( compiler, PREC_UNARY );
		case DENO_GRP: return CompileGroup( compiler );
		case DENO_PRE: return CompileUnary( compiler, &tk );
		case DENO_NUM: return CompileNum( compiler, &tk );
		case DENO_STR: return CompileStr( compiler, &tk );
		case DENO_REF: return CompileRef( compiler, &tk );
	}
}

static Expr CompilePostUnary( Compiler* compiler, Lexer* lexer, Expr src, Tk* tk ){
	Lex( lexer );
	Expr dst = ExprGen( EXPR_ERR, RegAlloc( compiler ) );
	printf( "CompilePostUnary: src: %d dst: %d tk: %d\n", src.reg, dst.reg, tk->type );
	/* Emit( &compiler->code, OpGet( POS_POST, tk->type ), dst, src,  ); */
	return dst;
}

static Expr CompilePostfix( Compiler* compiler, Expr src ){
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

static Expr CompileBinary( Compiler* compiler, Lexer* lexer, Expr lhs, Prec prec, Tk* tk ){
	Lex( lexer );
	Expr rhs = CompileExpr( compiler, prec );
	Expr dst = ExprGen( EXPR_ERR, RegAlloc( compiler ) );
	printf( "CompileBinary: lhs: %d tk: %d rhs: %d\n", lhs.reg, tk->type, rhs.reg );
	return dst;
}

static Expr CompileInfix( Compiler* compiler, Expr lhs, Prec min ){
	Lexer* lexer = compiler->lexer;
	Tk tk = lexer->tk; /* copy */
	while( DenoGet( PARSEPOS_INF, tk.type ) == DENO_INF ){
		Prec prec = PrecGet( tk.type );
		Assoc assoc = AssocGet( tk.type );
		if( prec - assoc <= min ) break;
		lhs = CompileBinary( compiler, lexer, lhs, prec, &tk );
		tk = lexer->tk;
	}
	return lhs;
}

static Expr CompileExpr( Compiler* compiler, Prec min ){
	Expr expr = CompilePrefix( compiler );
	expr = CompilePostfix( compiler, expr );
	expr = CompileInfix( compiler, expr, min );
	return expr;
}

static Expr CompileDecl( Compiler* compiler, Lexer* lexer, Tk* tk ){
	Lex( lexer ); /* eat colon */
	printf( "CompileDecl: %d\n", tk->intern );
	Expr rhs = CompileExpr( compiler, PREC_NONE );
	return rhs;
}

static Expr CompileId( Compiler* compiler ){
	Lexer* lexer = compiler->lexer;
	Tk tk = lexer->tk;
	Lex( lexer );
	if( lexer->tk.type == TK_COLON ) return CompileDecl( compiler, lexer, &tk );
	Expr ref = CompileRef( compiler, &tk ); /* continue parsing as expr. syntax is ambiguous. */
	ref = CompilePostfix( compiler, ref );
	ref = CompileInfix( compiler, ref, PREC_NONE );
	return ref;
}

static Expr CompileStmt( Compiler* compiler ){
	Lexer* lexer = compiler->lexer;
	switch( lexer->tk.type ){
		default: return CompileExpr( compiler, PREC_NONE );
		case TK_EOS: return ExprGen( EXPR_NULL, CMP_REG_ERR );
		case TK_ID: return CompileId( compiler );
		// case TK_IF: return CompileIf( compiler );
		// case TK_RET: return CompileReturn( compiler );
		// case TK_BREAK: return CompileBreak( compiler );
		// case TK_CONT: return CompileCont( compiler );
	}
}

Expr Compile( Compiler* compiler ){
	Lex( compiler->lexer );
	Expr e = CompileStmt( compiler );
	// Emit( &compiler->code, OP_HALT, 0, 0, 0 );
	return e;
}

// void CompilerFree( Compiler* compiler ){

// }