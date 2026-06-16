#include "compiler.h"

static Expr CompileExpr( Compiler* compiler, Prec min );
static Expr CompileStmt( Compiler* compiler );
static Expr CompileResolve( Compiler* compiler, Expr expr );

void CompilerInit( Compiler* compiler, Logs* logs, Lexer* lexer, Interns* interns, Consts* consts, Funcs* funcs, Syms* syms, FnSyms* fn_syms, Insts* insts ){
	compiler->logs = logs;
	compiler->lexer = lexer;
	compiler->interns = interns;
	compiler->consts = consts;
	compiler->funcs = funcs;
	compiler->syms = syms;
	compiler->fn_syms = fn_syms;
	compiler->insts = insts;
	InstInit( &compiler->scratch );
	compiler->scratch_ptr = 0;
	compiler->reg = 0;
	compiler->returned = 0;
}

void CompilerReset( Compiler* compiler ){
	compiler->reg = compiler->syms->len;
	compiler->returned = 0;
	compiler->scratch.len = 0;
	compiler->scratch_ptr = 0;
}

/* Temporary until we deal with blocks, scopes, functions, etc. */
static Reg RegAlloc( Compiler* compiler ){
	if( compiler->reg >= CMP_REG_CAP ){
		Throw( ERR_REGALLOC );
		return 0;
	}
	return ( Reg )compiler->reg++;
}

static Insts* CompilerInsts( Compiler* compiler ){
	if( compiler->scratch_ptr ) return compiler->scratch_ptr;
	return compiler->insts;
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
	InstABC( CompilerInsts( compiler ), op->code, ( u8 )dst.reg, ( u8 )src.reg, 0 );
	return dst;
}

static Expr CompileNum( Compiler* compiler, Tk* tk ){
	Expr dst = ExprGen( EXPR_NUM, RegAlloc( compiler ) );
	ConstId cid = ConstPutNum( compiler->consts, tk->num );
	InstAB( CompilerInsts( compiler ), OP_LOADC, ( u8 )dst.reg, ( u16 )cid );
	return dst;
}

static Expr CompileStr( Compiler* compiler, Tk* tk ){
	Expr dst = ExprGen( EXPR_STR, RegAlloc( compiler ) );
	ConstId cid = ConstPutStr( compiler->consts, tk->intern );
	InstAB( CompilerInsts( compiler ), OP_LOADC, ( u8 )dst.reg, ( u16 )cid );
	return dst;
}

static Expr CompileId( Tk* tk ){
	Expr expr = ExprGen( EXPR_ID, CMP_REG_ERR );
	expr.intern = tk->intern;
	return expr;
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
		case DENO_ID: return CompileId( &tk );
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
	src = CompileResolve( compiler, src );
	Op* op = OpGetPost( src.type, tk->type );
	if( !op->code ) return CompileBadPost( compiler, &src, tk );
	Expr dst = ExprGen( op->type, RegAlloc( compiler ) );
	InstABC( CompilerInsts( compiler ), op->code, ( u8 )dst.reg, ( u8 )src.reg, 0 );
	return dst;
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

static InstMark CompilerPushScratch( Compiler* compiler ){
	InstMark mark;
	mark.insts = compiler->scratch_ptr;
	mark.len = compiler->scratch.len;
	compiler->scratch_ptr = &compiler->scratch;
	return mark;
}

static void CompilerPopScratch( Compiler* compiler, InstMark mark ){
	compiler->scratch.len = mark.len;
	compiler->scratch_ptr = mark.insts;
}

static u8 CompileParams( Compiler* compiler, Lexer* lexer ){
	u8 nargs = 0;
	while( lexer->tk.type != TK_FNCLOSE && lexer->tk.type != TK_EOS ){
		Tk tk = lexer->tk;
		if( tk.type != TK_ID ){
			Log( compiler->logs, &tk.pos, CMP_BADPARAM );
			break;
		}
		Lex( lexer ); /* eat id */
		SymPut( compiler->syms, tk.intern, EXPR_NUM, nargs );
		nargs++;
		compiler->reg = nargs;
		if( lexer->tk.type == TK_COMMA ){
			Lex( lexer );
			continue;
		}
		break;
	}
	CompilerMatch( compiler, TK_FNCLOSE );
	return nargs;
}

static Expr CompileFn( Compiler* compiler, Lexer* lexer, Expr lhs, Tk* tk ){
	if( lhs.type != EXPR_ID ){
		Log( compiler->logs, &tk->pos, CMP_BADFNDECL );
		return ExprGen( EXPR_ERR, CMP_REG_ERR );
	}
	Lex( lexer ); /* eat <( */
	u8 nargs = CompileParams( compiler, lexer );
	FuncId fn_id = FuncPush( compiler->funcs );
	FnSymPut( compiler->fn_syms, lhs.intern, fn_id );
	u32 nsyms = compiler->syms->len; /* for restore */
	u32 nregs = compiler->reg; /* for restore */
	u8 returned = compiler->returned; /* for restore */
	compiler->reg = 0;
	compiler->returned = 0;
	InstMark mark = CompilerPushScratch( compiler );
	Expr last = CompileBlock( compiler, TK_END, TK_EOS );
	if( !compiler->returned && last.reg != CMP_REG_ERR ){
		InstABC( CompilerInsts( compiler ), OP_RET, ( u8 )last.reg, 0, 0 );
	}
	Func* fn = FuncGet( compiler->funcs, fn_id );
	fn->start = compiler->insts->len;
	fn->nargs = nargs;
	fn->ret_type = last.type;
	fn->nregs = compiler->reg;
	InstAppend( compiler->insts, &compiler->scratch, mark.len, compiler->scratch.len );
	fn->end = compiler->insts->len;
	CompilerPopScratch( compiler, mark );
	compiler->reg = nregs;
	compiler->syms->len = nsyms;
	compiler->returned = returned;
	return ExprGen( EXPR_NULL, CMP_REG_ERR );
}

static Expr CompileBadRef( Compiler* compiler, InternId intern ){
	Lexer* lexer = compiler->lexer;
	u8* ref = InternGet( compiler->interns, intern );
	Log( compiler->logs, &lexer->tk.pos, CMP_BADREF, ref );
	return ExprGen( EXPR_ERR, CMP_REG_ERR );
}

static u8 CompileArgs( Compiler* compiler, Lexer* lexer, Expr* args ){
	u8 nargs = 0;
	if( lexer->tk.type == TK_RP ){
		Lex( lexer );
		return 0;
	}
	for( ;; ){
		args[ nargs++ ] = CompileExpr( compiler, PREC_NONE );
		if( lexer->tk.type == TK_COMMA ){
			Lex( lexer );
			continue;
		}
		CompilerMatch( compiler, TK_RP );
		return nargs;
	}
}

static Expr CompileCall( Compiler* compiler, Lexer* lexer, Expr src, Tk* tk ){
	if( src.type != EXPR_ID ) return CompileBadPost( compiler, &src, tk );
	Lex( lexer ); /* eat ( */
	FnSym* sym = FnSymGet( compiler->fn_syms, src.intern );
	if( !sym ) return CompileBadRef( compiler, src.intern );
	Func* fn = FuncGet( compiler->funcs, sym->fn_id );
	Expr args[ CMP_ARG_CAP ];
	u8 nargs = CompileArgs( compiler, lexer, args );
	if( nargs != fn->nargs ){
		Log( compiler->logs, &tk->pos, CMP_BADARGC, fn->nargs, nargs );
		return ExprGen( EXPR_ERR, CMP_REG_ERR );
	}
	for( u8 i = 0; i < nargs; i++ ){
		if( args[ i ].type != EXPR_NUM ){
			Log( compiler->logs, &tk->pos, CMP_BADARGTYPE );
			return ExprGen( EXPR_ERR, CMP_REG_ERR );
		}
		InstABC( CompilerInsts( compiler ), OP_ARG, i, ( u8 )args[ i ].reg, 0 );
	}
	Expr dst = ExprGen( fn->ret_type, RegAlloc( compiler ) );
	InstAB( CompilerInsts( compiler ), OP_CALL, ( u8 )dst.reg, ( u16 )sym->fn_id );
	return dst;
}

static Expr CompilePostfix( Compiler* compiler, Expr src ){
	Lexer* lexer = compiler->lexer;
	for( ;; ){
		Tk tk = lexer->tk; /* copy required */
		Deno deno = DenoGet( PARSEPOS_POST, tk.type );
		switch( deno ){
			default: return src;
			case DENO_POST: src = CompilePost( compiler, lexer, src, &tk ); break;
			case DENO_FN: src = CompileFn( compiler, lexer, src, &tk ); break;
			case DENO_CALL: src = CompileCall( compiler, lexer, src, &tk ); break;
			// case DENO_MEMBER: src = CompileMember( compiler, lexer, src, &tk ); break;
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
	lhs = CompileResolve( compiler, lhs );
	Expr rhs = CompileExpr( compiler, prec );
	rhs = CompileResolve( compiler, rhs );
	Op* op = OpGetBinary( lhs.type, rhs.type, tk->type );
	if( !op->code ) return CompileBadBinary( compiler, &lhs, &rhs, tk );
	Expr dst = ExprGen( op->type, RegAlloc( compiler ) );
	InstABC( CompilerInsts( compiler ), op->code, ( u8 )dst.reg, ( u8 )lhs.reg, ( u8 )rhs.reg );
	return dst;
}

static Expr CompileDecl( Compiler* compiler, Lexer* lexer, Expr lhs, Prec prec, Tk* tk ){
	if( lhs.type != EXPR_ID ){
		Log( compiler->logs, &tk->pos, CMP_BADDECL );
		return ExprGen( EXPR_ERR, CMP_REG_ERR );
	}
	Lex( lexer ); /* eat : */
	Expr rhs = CompileExpr( compiler, prec );
	if( rhs.type == EXPR_ERR ) return rhs;
	SymPut( compiler->syms, lhs.intern, rhs.type, ( u8 )rhs.reg );
	return rhs;
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
			case DENO_DECL: lhs = CompileDecl( compiler, lexer, lhs, prec, &tk ); break;
		}
		tk = lexer->tk;
	}
	return lhs;
}

static Expr CompileBadRefType( Compiler* compiler, InternId intern ){
	Lexer* lexer = compiler->lexer;
	u8* ref = InternGet( compiler->interns, intern );
	Log( compiler->logs, &lexer->tk.pos, CMP_BADREFTYPE, ref );
	return ExprGen( EXPR_ERR, CMP_REG_ERR );
}

static Expr CompileResolve( Compiler* compiler, Expr expr ){
	if( expr.type != EXPR_ID ) return expr;
	Sym* sym = SymGet( compiler->syms, expr.intern );
	if( sym ) return ExprGen( sym->expr_type, sym->reg );
	FnSym* fn_sym = FnSymGet( compiler->fn_syms, expr.intern );
	if( fn_sym ) return CompileBadRefType( compiler, expr.intern );
	return CompileBadRef( compiler, expr.intern );
}

static Expr CompileExpr( Compiler* compiler, Prec min ){
	Expr expr = CompilePrefix( compiler );
	expr = CompilePostfix( compiler, expr );
	expr = CompileInfix( compiler, expr, min );
	expr = CompileResolve( compiler, expr );
	return expr;
}

static Expr CompileReturn( Compiler* compiler ){
	Lexer* lexer = compiler->lexer;
	Lex( lexer ); /* eat @ */
	Expr expr = CompileExpr( compiler, PREC_NONE );
	if( expr.type == EXPR_ERR ) return expr;
	InstABC( CompilerInsts( compiler ), OP_RET, ( u8 )expr.reg, 0, 0 );
	compiler->returned |= 1;
	return expr;
}

static Expr CompileStmt( Compiler* compiler ){
	Lexer* lexer = compiler->lexer;
	switch( lexer->tk.type ){
		default: return CompileExpr( compiler, PREC_NONE );
		case TK_EOS: return ExprGen( EXPR_NULL, CMP_REG_ERR );
		// case TK_IF: return CompileIf( compiler );
		case TK_RET: return CompileReturn( compiler );
		// case TK_BREAK: return CompileBreak( compiler );
		// case TK_CONT: return CompileCont( compiler );
	}
}

static FuncId CompilerFindFn( Compiler* compiler, u8* name ){
	InternId intern = InternPut( compiler->interns, name );
	FnSym* sym = FnSymGet( compiler->fn_syms, intern );
	if( !sym ) return FUNC_NONE;
	return sym->fn_id;
}

static FuncId CompileStart( Compiler* compiler, FuncId main_id ){
	Func* main = FuncGet( compiler->funcs, main_id );
	FuncId _start_id = FuncPush( compiler->funcs ); /* Compose _start that calls Main() */
	Func* _start = FuncGet( compiler->funcs, _start_id );
	_start->start = compiler->insts->len;
	_start->nargs = 0;
	_start->nregs = 1;
	_start->ret_type = main->ret_type;
	InstAB( CompilerInsts( compiler ), OP_CALL, 0, ( u16 )main_id ); /* Call Main() */
	InstABC( CompilerInsts( compiler ), OP_HALT, 0, 0, 0 );
	_start->end = compiler->insts->len;
	return _start_id;
}

FuncId Compile( Compiler* compiler ){
	Lexer* lexer = compiler->lexer;
	Lex( lexer );
	while( lexer->tk.type != TK_EOS ) CompileStmt( compiler );
	FuncId main_id = CompilerFindFn( compiler, ( u8* )"Main" );
	if( main_id == FUNC_NONE ){
		Log( compiler->logs, &lexer->tk.pos, CMP_NOMAIN );
		return FUNC_NONE;
	}
	return CompileStart( compiler, main_id );
}

void CompilerFree( Compiler* compiler ){
	InstFree( &compiler->scratch );
}