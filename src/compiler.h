#ifdef TYPES
typedef u32 Reg;

typedef struct Compiler {
	Logs* logs;
	Interns* interns;
	Consts* consts;
	Syms* syms;
	Insts* insts;
	Lexer* lexer;
	Reg reg;
} Compiler;
#endif

#ifdef IMPL
static Expr CompileExpr( Compiler* compiler, Prec min );
static Expr CompileStmt( Compiler* compiler, Lexer* lexer );

void CompilerInit( App* app, Compiler* compiler ){
	compiler->logs = &app->logs;
	compiler->interns = &app->interns;
	compiler->consts = &app->consts;
	compiler->syms = &app->syms;
	compiler->lexer = &app->lexer;
	compiler->insts = &app->insts;
	compiler->reg = 0;
}

static Reg RegAlloc( Compiler* compiler ){
	if( compiler->reg >= REG_CAP ){ Halt( ERR_REGALLOC ); }
	return ( Reg )compiler->reg++;
}

static void CompilerMatch( Compiler* compiler, TkType expected ){
	Lexer* lexer = compiler->lexer;
	Tk* tk = &lexer->tk;
	if( tk->type != expected ){
		Log( compiler->logs, &tk->pos, PARSE_EXPECT, expected, tk->type );
	}
	Lex( lexer );
}

static Expr CompileGroup( Compiler* compiler ){
	Expr src = CompileExpr( compiler, PREC_NONE );
	CompilerMatch( compiler, TK_RP );
	return src;
}

static Expr CompileBadUnary( Compiler* compiler, Expr* expr, Tk* tk ){
	u8* expr_name = ExprGetName( expr->type );
	u8* unary_name = OpGetUnaryName( tk->type );
	Log( compiler->logs, &tk->pos, CMP_BADUNARY, unary_name, expr_name );
	return ExprGen( EXPR_ERR, 0 );
}

static Expr CompileUnary( Compiler* compiler, Tk* tk ){
	Expr src = CompileExpr( compiler, PREC_UNARY );
	Op* op = OpGetUnary( src.type, tk->type );
	if( !op->code ) return CompileBadUnary( compiler, &src, tk );
	Expr dst = ExprGen( op->type, RegAlloc( compiler ) );
	InstABC( compiler->insts, op->code, dst.reg, src.reg, 0 );
	return dst;
}

static Expr CompileNum( Compiler* compiler, Tk* tk ){
	Expr dst = ExprGen( EXPR_NUM, RegAlloc( compiler ) );
	ConstIdx idx = ConstPutNum( compiler->consts, tk->num );
	InstABX( compiler->insts, OP_LOADC, dst.reg, idx );
	return dst;
}

static Expr CompileStr( Compiler* compiler, Tk* tk ){
	Expr dst = ExprGen( EXPR_STR, RegAlloc( compiler ) );
	ConstIdx idx = ConstPutStr( compiler->consts, tk->intern );
	InstABX( compiler->insts, OP_LOADC, dst.reg, idx );
	return dst;
}

static Expr CompileBadId( Compiler* compiler, Tk* tk ){
	u8* id = InternGetRaw( compiler->interns, tk->intern );
	Log( compiler->logs, &tk->pos, CMP_BADID, id );
	return ExprGen( EXPR_ERR, UINT32_MAX );
}

static Expr CompileId( Compiler* compiler, Tk* tk ){
	Sym* sym = SymGet( compiler->syms, tk->intern );
	if( !sym ) return CompileBadId( compiler, tk );
	return ExprGen( sym->expr_type, sym->reg );
}

static Expr CompileBadPrefix( Compiler* compiler, Deno deno, Tk* tk ){
	u8 *deno_name = DenoGetName( deno );
	u8 *tk_name = TkGetName( tk->type );
	Log( compiler->logs, &tk->pos, PARSE_BADPRE, deno_name, tk_name );
	return ExprGen( EXPR_ERR, 0 );
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
		case DENO_ID: return CompileId( compiler, &tk );
	}
}

static Expr CompileBadPost( Compiler* compiler, Expr* expr, Tk* tk ){
	u8* expr_name = ExprGetName( expr->type );
	u8* post_name = OpGetPostName( tk->type );
	Log( compiler->logs, &tk->pos, CMP_BADPOST, post_name, expr_name );
	return ExprGen( EXPR_ERR, UINT32_MAX );
}

static Expr CompilePost( Compiler* compiler, Lexer* lexer, Expr src, Tk* tk ){
	Lex( lexer );
	Op* op = OpGetPost( src.type, tk->type );
	if( !op->code ) return CompileBadPost( compiler, &src, tk );
	Expr dst = ExprGen( op->type, RegAlloc( compiler ) );
	InstABC( compiler->insts, op->code, dst.reg, src.reg, 0 );
	return dst;
}

static Expr CompileBlock( Compiler* compiler, TkType end1, TkType end2 ){
	Lexer* lexer = compiler->lexer;
	Expr last = ExprGen( EXPR_ERR, UINT32_MAX );
	while( lexer->tk.type != end1 && lexer->tk.type != end2 ){
		last = CompileStmt( compiler, lexer );
	}
	if( lexer->tk.type == end1 ){
		Lex( lexer );
		return last;
	}
	CompilerMatch( compiler, end1 ); /* Report missing end */
	return last;
}

static Expr CompilePostfix( Compiler* compiler, Expr src ){
	Lexer* lexer = compiler->lexer;
	for( ;; ){
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
	return ExprGen( EXPR_ERR, UINT32_MAX );
}

static Expr CompileBinary( Compiler* compiler, Lexer* lexer, Expr lhs, Prec prec, Tk* tk ){
	Lex( lexer );
	Expr rhs = CompileExpr( compiler, prec );
	Op* op = OpGetBinary( lhs.type, rhs.type, tk->type );
	if( !op->code ) return CompileBadBinary( compiler, &lhs, &rhs, tk );
	Expr dst = ExprGen( op->type, RegAlloc( compiler ) );
	InstABC( compiler->insts, op->code, dst.reg, lhs.reg, rhs.reg );
	return dst;
}

static Expr CompileInfix( Compiler* compiler, Expr lhs, Prec min ){
	Lexer* lexer = compiler->lexer;
	Tk tk = lexer->tk; /* copy */
	for( ;; ){
		Deno deno = DenoGet( PARSEPOS_INF, tk.type );
		if( !deno ) return lhs;
		Prec prec = PrecGet( tk.type );
		if( prec <= min ) break;
		prec -= AssocGet( tk.type );
		switch( deno ){
			default: return lhs;
			case DENO_INF: lhs = CompileBinary( compiler, lexer, lhs, prec, &tk ); break;
		}
		tk = lexer->tk;
	}
	return lhs;
}

static Expr CompileExprTail( Compiler* compiler, Expr lhs, Prec min ){
	lhs = CompilePostfix( compiler, lhs );
	lhs = CompileInfix( compiler, lhs, min );
	return lhs;
}

static Expr CompileExpr( Compiler* compiler, Prec min ){
	Expr head = CompilePrefix( compiler );
	return CompileExprTail( compiler, head, min );
}

static Expr CompileIdTail( Compiler* compiler, Tk* tk ){
	Expr id = CompileId( compiler, tk );
	return CompileExprTail( compiler, id, PREC_NONE );
}

static Expr CompileDecl( Compiler* compiler, Lexer* lexer ){
	Tk tk = lexer->tk;
	Lex( lexer ); /* eat id */
	if( lexer->tk.type != TK_ASSIGN ) return CompileIdTail( compiler, &tk );
	Lex( lexer ); /* eat : */
	Expr rhs = CompileExpr( compiler, PREC_NONE );
	if( rhs.type == EXPR_ERR ) return rhs;
	SymPut( compiler->syms, tk.intern, rhs.type, ( u8 )rhs.reg );
	return rhs;
}

static Expr CompileStmt( Compiler* compiler, Lexer* lexer ){
	switch( lexer->tk.type ){
		default: return CompileExpr( compiler, PREC_NONE );
		case TK_ID: return CompileDecl( compiler, lexer );
		case TK_EOS: return ExprGen( EXPR_ERR, UINT32_MAX );
	}
}

void CompilerRun( Compiler* compiler ){
	Lexer* lexer = compiler->lexer;
	Lex( lexer );
	Expr expr = ExprGen( EXPR_ERR, UINT32_MAX );
	while( lexer->tk.type != TK_EOS ) expr = CompileStmt( compiler, lexer );
	InstABC( compiler->insts, OP_HALT, expr.reg, 0, 0 );
}
#endif