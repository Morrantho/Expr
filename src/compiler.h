#ifdef TYPES
typedef u32 Reg;
#define REG_NONE UINT32_MAX

typedef enum ExprMode {			/* for compiling exprs as a stmt, or an expr. */
	EM_EXPR,
	EM_STMT
} ExprMode;

typedef struct Compiler {
	Logs* logs;
	Interns* interns;
	Consts* consts;
	Locals* locals;
	Insts* insts;
	Patches *ifs, *loops;
	Fns* fns;
	Lexer* lexer;
	/* state */
	Reg nregs;					/* regs in use */
	u32 nloops;					/* continue + break context */
	Reg dst;					/* for targeted exprs. needed for fn args. */
	ExprMode mode;				/* compiling exprs as exprs or stmts. changes bytecode. */
	FnIdx fn;					/* current fn */
} Compiler;
#endif

#ifdef IMPL
static Expr CompileExpr( Compiler* compiler, Lexer* lexer, Prec min );
static Expr CompileStmt( Compiler* compiler, Lexer* lexer );
static Expr CompileExprAs( Compiler* compiler, Lexer* lexer, Prec min, ExprMode mode );
static Expr CompileExprInto( Compiler* compiler, Lexer* lexer, Prec min, Reg new_dst );

static void CompilerReset( Compiler* compiler, FnIdx fn ){
	compiler->fn = fn;
	compiler->nregs = 0;
	compiler->nloops = 0;
	compiler->dst = REG_NONE;
	compiler->mode = EM_STMT;
	compiler->locals->len = 0;
}

void CompilerInit( Compiler* compiler, App* app ){
	compiler->logs = &app->logs;
	compiler->interns = &app->interns;
	compiler->consts = &app->consts;
	compiler->locals = &app->locals;
	compiler->insts = &app->insts;
	compiler->ifs = &app->ifs;
	compiler->loops = &app->loops;
	compiler->fns = &app->fns;
	compiler->lexer = &app->lexer;
	CompilerReset( compiler, FN_NONE );
}

static Reg RegAlloc( Compiler* compiler ){
	if( compiler->nregs >= REG_CAP ){ Halt( ERR_REGALLOC ); }
	return ( Reg )compiler->nregs++;
}

static Reg RegResult( Compiler* compiler ){
	if( compiler->dst == REG_NONE ){ return RegAlloc( compiler ); }
	return compiler->dst;
}

static Reg RegReserve( Compiler* compiler, Reg nregs ){
	if( !nregs ) nregs = 1;
	if( compiler->nregs + nregs > REG_CAP ){ Halt( ERR_REGALLOC ); }
	Reg reg = compiler->nregs;
	compiler->nregs += nregs;
	return reg;
}

static void CompilerMatch( Compiler* compiler, Lexer* lexer, TkType expected ){
	Tk* tk = &lexer->tk;
	if( tk->type != expected ){
		Log( compiler->logs, &tk->pos, PARSE_EXPECT, TkGetName( expected ), TkGetName( tk->type ) );
	}
	Lex( lexer );
}

static Expr CompileVoid( Compiler* compiler ){
	Expr expr = ExprAs( EXPR_VOID, RegResult( compiler ) );
	InstABX( compiler->insts, OP_LOADC, expr.reg, CONST_VOID );
	return expr;
}

static Expr CompileBadId( Compiler* compiler, SrcPos* pos, InternIdx intern ){
	u8* id = InternGetRaw( compiler->interns, intern );
	Log( compiler->logs, pos, CMP_BADID, id );
	return ExprErr( );
}

static Expr CompileReg( Compiler* compiler, Expr expr, SrcPos* pos ){
	if( expr.type != EXPR_ID ){ return expr; }
	Local* local = LocalFind( compiler->locals, expr.intern );
	if( local ){ return ExprAs( local->expr_type, local->reg ); }
	FnIdx fn_idx = FnFind( compiler->fns, expr.intern );
	if( fn_idx == FN_NONE ) return CompileBadId( compiler, pos, expr.intern );
	Expr dst = ExprAs( EXPR_VALUE, RegResult( compiler ) );
	ConstIdx con_idx = ConstPutFn( compiler->consts, fn_idx );
	InstABX( compiler->insts, OP_LOADC, dst.reg, con_idx );
	return dst;
}

static Expr CompileBadPrefix( Compiler* compiler, Deno deno, Tk* tk ){
	u8 *deno_name = DenoGetName( deno );
	u8 *tk_name = TkGetName( tk->type );
	Log( compiler->logs, &tk->pos, PARSE_BADPRE, deno_name, tk_name );
	return ExprErr( );
}

static Expr CompileGroup( Compiler* compiler, Lexer* lexer ){
	Expr src = CompileExpr( compiler, lexer, PREC_NONE );
	CompilerMatch( compiler, lexer, TK_RP );
	return src;
}

static Expr CompileBadUnary( Compiler* compiler, Expr* expr, Tk* tk ){
	u8* expr_name = ExprGetName( expr->type );
	u8* unary_name = TkGetName( tk->type );
	Log( compiler->logs, &tk->pos, CMP_BADUNARY, unary_name, expr_name );
	return ExprErr( );
}
/* Avoids regalloc for mutating unarys in stmt position. */
static u8 CompileUnaryStmt( Compiler* compiler, Op* op, Expr* src ){
	if( compiler->mode != EM_STMT || !op->mut ){ return 0; }
	InstABC( compiler->insts, op->code, src->reg, src->reg, 0 ); /* no alloc, self-mutate */
	return 1;
}

static Expr CompileUnary( Compiler* compiler, Lexer* lexer, Tk* tk ){
	Expr src = CompileExprAs( compiler, lexer, PREC_UNARY, EM_EXPR );
	Op* op = OpGetUnary( tk->type );
	if( op->code == OP_ERR ){ return CompileBadUnary( compiler, &src, tk ); }
	if( CompileUnaryStmt( compiler, op, &src ) ){ return ExprVoid( ); }
	Expr dst = ExprAs( EXPR_VALUE, RegResult( compiler ) );
	InstABC( compiler->insts, op->code, dst.reg, src.reg, 0 );
	return dst;
}

static Expr CompileNum( Compiler* compiler, Tk* tk ){
	Expr dst = ExprAs( EXPR_VALUE, RegResult( compiler ) );
	ConstIdx idx = ConstPutNum( compiler->consts, tk->num );
	InstABX( compiler->insts, OP_LOADC, dst.reg, idx );
	return dst;
}

static Expr CompileStr( Compiler* compiler, Tk* tk ){
	Expr dst = ExprAs( EXPR_VALUE, RegResult( compiler ) );
	ConstIdx idx = ConstPutStr( compiler->consts, tk->intern );
	InstABX( compiler->insts, OP_LOADC, dst.reg, idx );
	return dst;
}

static Expr CompilePrefix( Compiler* compiler, Lexer* lexer ){
	Tk tk = lexer->tk; /* copy */
	Deno deno = DenoGet( PARSEPOS_PRE, tk.type );
	Lex( lexer );
	switch( deno ){
		default: return CompileBadPrefix( compiler, deno, &tk );
		case DENO_NOPPRE: return CompileExprAs( compiler, lexer, PREC_UNARY, EM_EXPR );
		case DENO_GRP: return CompileGroup( compiler, lexer );
		case DENO_PRE: return CompileUnary( compiler, lexer, &tk );
		case DENO_NUM: return CompileNum( compiler, &tk );
		case DENO_STR: return CompileStr( compiler, &tk );
		case DENO_ID: return ExprId( tk.intern );
	}
}

static Expr CompileBadPost( Compiler* compiler, Expr* src, Tk* tk ){
	u8* src_name = ExprGetName( src->type );
	u8* post_name = TkGetName( tk->type );
	Log( compiler->logs, &tk->pos, CMP_BADPOST, post_name, src_name );
	return ExprErr( );
}

static u8 CompilePostStmt( Compiler* compiler, Op* op, Expr* src, Tk* tk ){
	if( compiler->mode != EM_STMT || !op->mut ){ return 0; }
	Op* mut = OpGetUnary( tk->type ); /* use prefix, less vm overhead */
	InstABC( compiler->insts, mut->code, src->reg, src->reg, 0 ); /* no alloc, self-mutate. */
	return 1;
}

static Expr CompilePost( Compiler* compiler, Lexer* lexer, Expr src, Tk* tk ){
	src = CompileReg( compiler, src, &tk->pos );
	Lex( lexer );
	Op* op = OpGetPost( tk->type );
	if( op->code == OP_ERR ){ return CompileBadPost( compiler, &src, tk ); }
	if( CompilePostStmt( compiler, op, &src, tk ) ){ return ExprVoid( ); }
	Expr dst = ExprAs( EXPR_VALUE, RegResult( compiler ) );
	InstABC( compiler->insts, op->code, dst.reg, src.reg, 0 );
	return dst;
}

static Expr CompileCallArgs( Compiler* compiler, Lexer* lexer, Reg base, u8* nargs ){
	*nargs = 0;
	Lex( lexer ); /* ( */
	if( lexer->tk.type == TK_RP ){ Lex( lexer ); return ExprVoid( ); }
	for( ;; ){
		if( *nargs == UINT8_MAX ){ Halt( ERR_REGALLOC ); }
		Reg arg = RegReserve( compiler, 1 );
		if( arg != base + 1 + *nargs ){ Halt( ERR_REGALLOC ); } /* Bail if not packed */
		Expr expr = CompileExprInto( compiler, lexer, PREC_NONE, arg );
		if( expr.type == EXPR_ERR ) return expr;
		( *nargs )++;
		compiler->nregs = arg + 1;
		if( lexer->tk.type != TK_COMMA ) break;
		Lex( lexer );
	}
	CompilerMatch( compiler, lexer, TK_RP ); /* ) */
	return ExprVoid( );
}

static Expr CompileMoveTo( Compiler* compiler, Expr expr, Reg new_dst ){
	if( expr.type == EXPR_ERR || expr.type == EXPR_VOID || expr.reg == new_dst ){ return expr; }
	InstABC( compiler->insts, OP_MOV, new_dst, expr.reg, 0 );
	return ExprAs( expr.type, new_dst );
}

static Expr CompileCall( Compiler* compiler, Lexer* lexer, Expr src ){
	Tk tk = lexer->tk;
	src = CompileReg( compiler, src, &tk.pos );
	if( src.type == EXPR_ERR ) return src;
	if( src.type == EXPR_VOID ) return CompileBadPost( compiler, &src, &tk );
	Reg base = RegReserve( compiler, 1 );
	src = CompileMoveTo( compiler, src, base );
	u8 nargs = 0;
	Expr args = CompileCallArgs( compiler, lexer, base, &nargs );
	if( args.type == EXPR_ERR ) return args;
	InstABC( compiler->insts, OP_CALL, base, nargs, 0 );
	compiler->nregs = base + 1;
	return ExprAs( EXPR_VALUE, base );
}

static Expr CompilePostfix( Compiler* compiler, Lexer* lexer, Expr src ){
	for( ;; ){
		Tk tk = lexer->tk; /* copy required */
		Deno deno = DenoGet( PARSEPOS_POST, tk.type );
		switch( deno ){
			default: return src;
			case DENO_POST: src = CompilePost( compiler, lexer, src, &tk ); break;
			case DENO_CALL: src = CompileCall( compiler, lexer, src ); break;
		}
	}
}

static Expr CompileBadBinary( Compiler* c, Expr* lhs, Expr* rhs, Tk* tk ){
	u8* lhs_type = ExprGetName( lhs->type );
	u8* rhs_type = ExprGetName( rhs->type );
	u8* bin_name = TkGetName( tk->type );
	Log( c->logs, &tk->pos, CMP_BADBINARY, bin_name, lhs_type, rhs_type );
	return ExprErr( );
}

static u8 CompileBinaryStmt( Compiler* compiler, Op* op, Expr* lhs, Expr* rhs ){
	if( compiler->mode != EM_STMT || !op->mut ){ return 0; }
	InstABC( compiler->insts, op->code, lhs->reg, lhs->reg, rhs->reg ); /* no alloc, self-mutate */
	return 1;
}

static Expr CompileBinary( Compiler* compiler, Lexer* lexer, Expr lhs, Prec prec, Tk* tk ){
	Lex( lexer );
	Expr rhs = CompileExprAs( compiler, lexer, prec, EM_EXPR );
	Op* op = OpGetBinary( tk->type );
	if( op->code == OP_ERR ){ return CompileBadBinary( compiler, &lhs, &rhs, tk ); }
	if( CompileBinaryStmt( compiler, op, &lhs, &rhs ) ){ return ExprVoid( ); }
	Expr dst = ExprAs( EXPR_VALUE, RegResult( compiler ) );
	InstABC( compiler->insts, op->code, dst.reg, lhs.reg, rhs.reg );
	return dst;
}

static Expr CompileInfix( Compiler* compiler, Lexer* lexer, Expr lhs, Prec min ){
	Tk tk = lexer->tk;
	lhs = CompileReg( compiler, lhs, &tk.pos );
	for( ;; ){
		tk = lexer->tk; /* copy */
		if( DenoGet( PARSEPOS_INF, tk.type ) != DENO_INF ){ return lhs; }
		Prec prec = PrecGet( tk.type );
		if( prec <= min ){ return lhs; }
		prec -= AssocGet( tk.type );
		lhs = CompileBinary( compiler, lexer, lhs, prec, &tk );
	}
	return lhs;
}

static Expr CompileExpr( Compiler* compiler, Lexer* lexer, Prec min ){
	Expr expr = CompilePrefix( compiler, lexer );
	expr = CompilePostfix( compiler, lexer, expr );
	expr = CompileInfix( compiler, lexer, expr, min );
	return expr;
}
/* Compile an expr as a stmt or an expr. */
static Expr CompileExprAs( Compiler* compiler, Lexer* lexer, Prec min, ExprMode new_mode ){
	ExprMode old_mode = compiler->mode;
	Reg old_dst = compiler->dst;
	compiler->mode = new_mode;
	compiler->dst = REG_NONE;
	Expr expr = CompileExpr( compiler, lexer, min );
	expr = CompileReg( compiler, expr, &lexer->tk.pos );
	compiler->mode = old_mode;
	compiler->dst = old_dst;
	return expr;
}

/* compile an expr into a specific register. */
static Expr CompileExprInto( Compiler* compiler, Lexer* lexer, Prec min, Reg new_dst ){
	ExprMode old_mode = compiler->mode;
	Reg old_dst = compiler->dst;
	compiler->mode = EM_EXPR;
	compiler->dst = new_dst;
	Expr expr = CompileExpr( compiler, lexer, min );
	expr = CompileReg( compiler, expr, &lexer->tk.pos );
	expr = CompileMoveTo( compiler, expr, new_dst );
	compiler->mode = old_mode;
	compiler->dst = old_dst;
	return expr;
}

static void CompileLoopBody( Compiler* compiler, Lexer* lexer ){
	compiler->nloops++;
	u32 old_locals = compiler->locals->len;
	while( lexer->tk.type != TK_END && lexer->tk.type != TK_EOS )
		{ CompileStmt( compiler, lexer ); }
	compiler->locals->len = old_locals;
	compiler->nloops--;
}

static Expr CompileLoop( Compiler* compiler, Lexer* lexer ){
	Lex( lexer ); /* eat ;; */
	PatchIdx mark = compiler->loops->len;
	InstIdx enter = compiler->insts->len;
	CompileLoopBody( compiler, lexer );
	InstJmp( compiler->insts, enter );
	InstIdx exit = compiler->insts->len;
	PatchLoop( compiler->loops, mark, enter, exit );
	CompilerMatch( compiler, lexer, TK_END );
	return ExprVoid( );
}

static Expr CompileBadControl( Compiler* compiler, SrcPos* pos, LogType type ){
	Log( compiler->logs, pos, type );
	return ExprErr( );
}

/* continue and break */
static Expr CompileControl( Compiler* compiler, Lexer* lexer, PatchType patch_type, LogType log_type ){
	Tk tk = lexer->tk;
	Lex( lexer ); /* ==> or <== */
	if( !compiler->nloops ){ return CompileBadControl( compiler, &tk.pos, log_type ); }
	PatchPush( compiler->loops, patch_type, InstJmp( compiler->insts, 0 ) );
	return ExprVoid( );
}

static Expr CompileBreak( Compiler* compiler, Lexer* lexer ){
	return CompileControl( compiler, lexer, PATCH_BREAK, CMP_BADBRK );
}

static Expr CompileContinue( Compiler* compiler, Lexer* lexer ){
	return CompileControl( compiler, lexer, PATCH_CONTINUE, CMP_BADCONT );
}

static void CompileIfBody( Compiler* compiler, Lexer* lexer ){
	u32 old_locals = compiler->locals->len;
	while( lexer->tk.type != TK_END 
		&& lexer->tk.type != TK_ELIF
		&& lexer->tk.type != TK_ELSE
		&& lexer->tk.type != TK_EOS )
			{ CompileStmt( compiler, lexer ); }
	compiler->locals->len = old_locals;
}

static PatchIdx CompileIfCond( Compiler* compiler, Lexer* lexer ){
	PatchIdx branch = compiler->ifs->len;
	Lex( lexer ); /* eat ?( or ??( */
	Expr cond = CompileExprAs( compiler, lexer, PREC_NONE, EM_EXPR );
	CompilerMatch( compiler, lexer, TK_RP ); /* match ) */
	PatchPush( compiler->ifs, PATCH_BRANCH, InstJz( compiler->insts, cond.reg ) );
	return branch;
}

static u8 CompileIfLast( Compiler* compiler, Lexer* lexer, PatchIdx branch ){
	if( lexer->tk.type == TK_ELIF || lexer->tk.type == TK_ELSE ){ return 0; }
	PatchApply( compiler->ifs, PATCH_BRANCH, branch, compiler->insts->len );
	return 1;
}

static void CompileIfBranch( Compiler* compiler, Lexer* lexer ){
	PatchIdx branch = CompileIfCond( compiler, lexer );
	CompileIfBody( compiler, lexer );
	if( CompileIfLast( compiler, lexer, branch ) ){ return; }
	InstIdx end = InstJmp( compiler->insts, 0 );
	PatchApply( compiler->ifs, PATCH_BRANCH, branch, compiler->insts->len );
	PatchPush( compiler->ifs, PATCH_END, end );
}

static void CompileElse( Compiler* compiler, Lexer* lexer ){
	if( lexer->tk.type != TK_ELSE ){ return; }
	Lex( lexer ); /* ?? */
	CompileIfBody( compiler, lexer );
}

static Expr CompileIf( Compiler* compiler, Lexer* lexer ){
	PatchIdx mark = compiler->ifs->len;
	do{ CompileIfBranch( compiler, lexer ); } while( lexer->tk.type == TK_ELIF );
	CompileElse( compiler, lexer );
	CompilerMatch( compiler, lexer, TK_END );
	PatchApply( compiler->ifs, PATCH_END, mark, compiler->insts->len );
	return ExprVoid( );
}

static u32 CompileFnArgs( Compiler* compiler, Lexer* lexer, InternIdx* args ){
	u32 nargs = 0;
	while( lexer->tk.type != TK_FNCLOSE && lexer->tk.type != TK_EOS ){
		if( nargs >= UINT8_MAX ) Halt( ERR_REGALLOC );
		InternIdx arg = lexer->tk.intern;
		CompilerMatch( compiler, lexer, TK_ID );
		args[ nargs++ ] = arg;
		if( lexer->tk.type != TK_COMMA ){ break; }
		Lex( lexer );
	}
	return nargs;
}

static void CompileSkipBody( Compiler* compiler, Lexer* lexer, SrcPos* pos ){
	*pos = lexer->tk.pos;
	for( u32 depth = 1; depth; Lex( lexer ) ){
		switch( lexer->tk.type ){
			case TK_EOS:
				Log( compiler->logs, &lexer->tk.pos, PARSE_EXPECT, TkGetName( TK_END ), TkGetName( TK_EOS ) );
				return;
			case TK_IF: case TK_LOOP: case TK_FNOPEN: depth++; break;
			case TK_END: depth--; break;
			default: break;
		}
	}
}

static Expr CompileFn( Compiler* compiler, Lexer* lexer, InternIdx name ){
	CompilerMatch( compiler, lexer, TK_FNOPEN ); /* <( */
	InternIdx args[ REG_CAP ];
	u32 nargs = CompileFnArgs( compiler, lexer, args );
	CompilerMatch( compiler, lexer, TK_FNCLOSE ); /* )> */
	SrcPos body;
	CompileSkipBody( compiler, lexer, &body );
	ArgIdx arg_base = FnArgsPush( compiler->fns, args, nargs );
	FnPush( compiler->fns, name, &body, arg_base, nargs );
	return ExprVoid( );
}

static void CompileFnArgsInto( Compiler* compiler, Fn* fn ){
	InternIdx* args = FnArgsGet( compiler->fns, fn );
	compiler->nregs = fn->nargs;
	for( u8 i = 0; i < fn->nargs; i++ )
		{ LocalPut( compiler->locals, args[ i ], EXPR_VALUE, i ); }
}

static void CompileFnBody( Compiler* compiler, Fn* fn ){
	Lexer lexer = *compiler->lexer; /* copy lexer */
	LexAt( &lexer, &fn->body ); /* parse from fn entry */
	while( lexer.tk.type != TK_END && lexer.tk.type != TK_EOS )
		{ CompileStmt( compiler, &lexer ); }	
}

static void CompileFnCode( Compiler* compiler, FnIdx fn_idx ){
	Fn* fn = FnGet( compiler->fns, fn_idx );
	if( fn->target != INST_NONE ){ return; } /* Should catch natives */
	CompilerReset( compiler, fn_idx );
	fn->target = compiler->insts->len;
	CompileFnArgsInto( compiler, fn );
	CompileFnBody( compiler, fn );
	Expr ret = CompileVoid( compiler );
	InstABC( compiler->insts, OP_RET, ret.reg, 0, 0 );
	fn = FnGet( compiler->fns, fn_idx ); /* Reacquire */
	fn->nregs = compiler->nregs;
	CompilerReset( compiler, FN_NONE );
}

static void CompileFns( Compiler* compiler ){
	for( FnIdx i = 0; i < compiler->fns->fn_len; i++ ){
		CompileFnCode( compiler, i );
	}
}

static Expr CompileDecl( Compiler* compiler, Lexer* lexer ){
	if( lexer->peek.type != TK_ASSIGN ){ return CompileExprAs( compiler, lexer, PREC_NONE, EM_STMT ); }
	Tk tk = lexer->tk;
	Lex( lexer ); /* eat id */
	Lex( lexer ); /* eat : */
	if( lexer->tk.type == TK_FNOPEN ){ return CompileFn( compiler, lexer, tk.intern ); }
	Expr rhs = CompileExprAs( compiler, lexer, PREC_NONE, EM_EXPR );
	if( rhs.type == EXPR_ERR ){ return rhs; }
	LocalPut( compiler->locals, tk.intern, EXPR_VALUE, rhs.reg );
	return ExprAs( EXPR_VALUE, rhs.reg );
}

static Expr CompileBadReturn( Compiler* compiler, Tk* tk ){
	Log( compiler->logs, &tk->pos, CMP_BADRET );
	return ExprErr( );
}

static Expr CompileReturn( Compiler* compiler, Lexer* lexer ){
	Tk tk = lexer->tk;
	Lex( lexer ); /* <- */
	if( compiler->fn == FN_NONE ){ return CompileBadReturn( compiler, &tk ); }
	Expr ret = CompileExprAs( compiler, lexer, PREC_NONE, EM_EXPR );
	if( ret.type == EXPR_ERR ){ return ret; }
	InstABC( compiler->insts, OP_RET, ret.reg, 0, 0 );
	return ExprVoid( );
}

static Expr CompileStmt( Compiler* compiler, Lexer* lexer ){
	switch( lexer->tk.type ){
		default: return CompileExprAs( compiler, lexer, PREC_NONE, EM_STMT );
		case TK_LOOP: return CompileLoop( compiler, lexer );
		case TK_BREAK: return CompileBreak( compiler, lexer );
		case TK_CONT: return CompileContinue( compiler, lexer );
		case TK_IF: return CompileIf( compiler, lexer );
		case TK_RET: return CompileReturn( compiler, lexer );
		case TK_ID: return CompileDecl( compiler, lexer );
		case TK_EOS: return ExprErr( );
	}
}

InstIdx CompilerRun( Compiler* compiler ){
	Lexer* lexer = compiler->lexer;
	InstIdx entry = compiler->insts->len;
	CompilerReset( compiler, FN_NONE );
	Expr expr = ExprVoid( );
	while( lexer->tk.type != TK_EOS ){ expr = CompileStmt( compiler, lexer ); }
	if( expr.type == EXPR_VOID ){ expr = CompileVoid( compiler ); }
	InstABC( compiler->insts, OP_HALT, expr.reg, 0, 0 );
	CompileFns( compiler );
	return entry;
}
#endif