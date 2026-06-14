#include "compiler.h"

static Expr CompileExpr( Compiler* compiler, Prec min );
static Expr CompileStmt( Compiler* compiler );

void CompilerInit( Compiler* compiler, Logs* logs, Lexer* lexer, Interns* interns, Consts* consts, Funcs* funcs, Syms* syms, FnSyms* fn_syms, Insts* insts ){
	compiler->logs = logs;
	compiler->lexer = lexer;
	compiler->interns = interns;
	compiler->consts = consts;
	compiler->funcs = funcs;
	compiler->syms = syms;
	compiler->fn_syms = fn_syms;
	compiler->insts = insts;
	compiler->reg = 0;
}

void CompilerReset( Compiler* compiler ){
	compiler->reg = compiler->syms->len;
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

static Expr CompileBadUnary( Compiler* c, Expr* expr, Tk* tk ){
	u8* expr_name = ExprGetName( expr->type );
	u8* unary_name = OpGetUnaryName( tk->type );
	Log( c->logs, &tk->pos, CMP_BADUNARY, unary_name, expr_name );
	return ExprGen( EXPR_ERR, CMP_REG_ERR );
}

static Expr CompileUnary( Compiler* compiler, Tk* tk ){
	Expr src = CompileExpr( compiler, PREC_UNARY );
	Op* op = OpGetUnary( src.type, tk->type );
	if( !op->code ) return CompileBadUnary( compiler, &src, tk );
	Expr dst = ExprGen( op->type, RegAlloc( compiler ) );
	InstABC( compiler->insts, op->code, ( u8 )dst.reg, ( u8 )src.reg, 0 );
	return dst;
}

static Expr CompileNum( Compiler* compiler, Tk* tk ){
	Expr dst = ExprGen( EXPR_NUM, RegAlloc( compiler ) );
	ConstId cid = ConstPutNum( compiler->consts, tk->num );
	InstAB( compiler->insts, OP_LOADC, ( u8 )dst.reg, ( u16 )cid );
	return dst;
}

static Expr CompileStr( Compiler* compiler, Tk* tk ){
	Expr dst = ExprGen( EXPR_STR, RegAlloc( compiler ) );
	ConstId cid = ConstPutStr( compiler->consts, tk->intern );
	InstAB( compiler->insts, OP_LOADC, ( u8 )dst.reg, ( u16 )cid );
	return dst;
}

static Expr CompileBadRef( Compiler* compiler, Tk* tk ){
	u8* ref = InternGet( compiler->interns, tk->intern );
	Log( compiler->logs, &tk->pos, CMP_BADREF, ref );
	return ExprGen( EXPR_ERR, CMP_REG_ERR );
}

static Expr CompileRef( Compiler* compiler, Tk* tk ){
	Sym* sym = SymGet( compiler->syms, tk->intern );
	if( !sym ) return CompileBadRef( compiler, tk );
	return ExprGen( sym->expr_type, sym->reg );
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

static Expr CompileBadPost( Compiler* c, Expr* expr, Tk* tk ){
	u8* expr_name = ExprGetName( expr->type );
	u8* post_name = OpGetPostName( tk->type );
	Log( c->logs, &tk->pos, CMP_BADPOST, post_name, expr_name );
	return ExprGen( EXPR_ERR, CMP_REG_ERR );
}

static Expr CompilePost( Compiler* compiler, Lexer* lexer, Expr src, Tk* tk ){
	Lex( lexer );
	Op* op = OpGetPost( src.type, tk->type );
	if( !op->code ) return CompileBadPost( compiler, &src, tk );
	Expr dst = ExprGen( op->type, RegAlloc( compiler ) );
	InstABC( compiler->insts, op->code, ( u8 )dst.reg, ( u8 )src.reg, 0 );
	return dst;
}

static Expr CompilePostfix( Compiler* compiler, Expr src ){
	Lexer* lexer = compiler->lexer;
	for( ;; ){ /* member access + calls can chain. not yet implemented. */
		Tk tk = lexer->tk; /* copy required */
		Deno deno = DenoGet( PARSEPOS_POST, tk.type );
		switch( deno ){
			default: return src;
			case DENO_POST: src = CompilePost( compiler, lexer, src, &tk ); break;
		}
	}
}

static Expr CompileBadBinary( Compiler* c, Expr* lhs, Expr* rhs, Tk* tk ){
	u8* lhs_type = ExprGetName( lhs->type );
	u8* rhs_type = ExprGetName( rhs->type );
	u8* bin_name = OpGetBinaryName( tk->type );
	Log( c->logs, &tk->pos, CMP_BADBINARY, bin_name, lhs_type, rhs_type );
	return ExprGen( EXPR_ERR, CMP_REG_ERR );
}

static Expr CompileBinary( Compiler* compiler, Lexer* lexer, Expr lhs, Prec prec, Tk* tk ){
	Lex( lexer );
	Expr rhs = CompileExpr( compiler, prec );
	Op* op = OpGetBinary( lhs.type, rhs.type, tk->type );
	if( !op->code ) return CompileBadBinary( compiler, &lhs, &rhs, tk );
	Expr dst = ExprGen( op->type, RegAlloc( compiler ) );
	InstABC( compiler->insts, op->code, ( u8 )dst.reg, ( u8 )lhs.reg, ( u8 )rhs.reg );
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
	Expr rhs = CompileExpr( compiler, PREC_NONE );
	if( rhs.type == EXPR_ERR ) return rhs;
	SymPut( compiler->syms, tk->intern, rhs.type, ( u8 )rhs.reg );
	return rhs;
}

static Expr CompileBlock( Compiler* compiler, TkType end1, TkType end2 ){
	Lexer* lexer = compiler->lexer;
	Expr last = ExprGen( EXPR_NULL, CMP_REG_ERR );
	while( lexer->tk.type != end1 && lexer->tk.type != end2 ){
		last = CompileStmt( compiler );
	}
	if( lexer->tk.type == end1 ){
		Lex( lexer );
		return last;
	}
	CompilerMatch( compiler, end1 ); /* Report missing end */
	return last;
}

static Expr CompileFn( Compiler* compiler, Lexer* lexer, Tk* tk ){
	Lex( lexer ); /* eat <( */
	/* u32 nargs = CompileArgs( compiler ); */
	CompilerMatch( compiler, TK_FNCLOSE ); /* )> */
	FuncId fn_id = FuncPush( compiler->funcs );
	FnSymPut( compiler->fn_syms, tk->intern, fn_id );
	u32 nsyms = compiler->syms->len; /* so we can revert */
	u32 nregs = compiler->reg;
	compiler->reg = 0; /* so we can count regs consumed */
	Func* fn = FuncGet( compiler->funcs, fn_id );
	fn->start = compiler->insts->len;
	fn->nargs = 0;
	Expr last = CompileBlock( compiler, TK_END, TK_EOS );
	if( last.reg != CMP_REG_ERR ) InstABC( compiler->insts, OP_RET, last.reg, 0, 0 );
	fn->end = compiler->insts->len;
	fn->nregs = compiler->reg;
	compiler->reg = nregs; /* Restore to former state */
	compiler->syms->len = nsyms;
	return ExprGen( EXPR_NULL, CMP_REG_ERR );
}

static Expr CompileId( Compiler* compiler ){
	Lexer* lexer = compiler->lexer;
	Tk tk = lexer->tk;
	Lex( lexer );
	if( lexer->tk.type == TK_ASSIGN ) return CompileDecl( compiler, lexer, &tk );
	if( lexer->tk.type == TK_FNOPEN ) return CompileFn( compiler, lexer, &tk );
	Expr ref = CompileRef( compiler, &tk ); /* continue parsing as expr. syntax is ambiguous. */
	ref = CompilePostfix( compiler, ref );
	ref = CompileInfix( compiler, ref, PREC_NONE );
	return ref;
}

static Expr CompileReturn( Compiler* compiler ){
	Lexer* lexer = compiler->lexer;
	Lex( lexer ); /* eat @ */
	Expr expr = CompileExpr( compiler, PREC_NONE );
	if( expr.type == EXPR_ERR ) return expr;
	InstABC( compiler->insts, OP_RET, ( u8 )expr.reg, 0, 0 );
	return expr;
}

static Expr CompileStmt( Compiler* compiler ){
	Lexer* lexer = compiler->lexer;
	switch( lexer->tk.type ){
		default: return CompileExpr( compiler, PREC_NONE );
		case TK_EOS: return ExprGen( EXPR_NULL, CMP_REG_ERR );
		case TK_ID: return CompileId( compiler );
		// case TK_IF: return CompileIf( compiler );
		case TK_RET: return CompileReturn( compiler );
		// case TK_BREAK: return CompileBreak( compiler );
		// case TK_CONT: return CompileCont( compiler );
	}
}

FuncId CompilerFindFn( Compiler* compiler, u8* name ){
	InternId intern = InternPut( compiler->interns, name );
	FnSym* sym = FnSymGet( compiler->fn_syms, intern );
	if( !sym ) return FUNC_NONE;
	return sym->fn_id;
}

FuncId Compile( Compiler* compiler ){
	Lexer* lexer = compiler->lexer;
	Lex( lexer );
	while( lexer->tk.type != TK_EOS ) CompileStmt( compiler );
	FuncId entry = CompilerFindFn( compiler, ( u8* )"Main" );
	if( entry == FUNC_NONE ) Log( compiler->logs, &lexer->tk.pos, CMP_NOMAIN );
	return entry;
}