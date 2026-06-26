#ifdef TYPES
typedef u32 Reg;
#define REG_NONE UINT32_MAX

typedef enum ExprMode {			/* for compiling exprs as a stmt, or an expr. */
	EM_EXPR,
	EM_STMT
} ExprMode;
/* for compiler-specific state save and restore. unrelated to vm frames. */
typedef struct CompilerFrame {	/* register span */
	Reg reg;					/* base reg */
	Reg dst;					/* destination reg */
	ChunkIdx chunk;				/* previous chunk */
	u32 nlocals;
	ExprMode mode;				/* as stmt or as expr */
	FnImplIdx impl;
} CompilerFrame;

typedef struct Compiler {
	Logs* logs;
	Interns* interns;
	Consts* consts;
	Locals* locals;
	Insts* insts;
	Chunks* chunks;
	Patches *ifs, *loops;
	Fns* fns;
	Lexer* lexer;
	/* state */
	Reg nregs;					/* regs in use */
	u32 nloops;					/* continue + break context */
	Reg dst;					/* for targeted exprs. needed for fn args. */
	ExprMode mode;				/* compiling exprs as exprs or stmts. changes bytecode. */
	ChunkIdx chunk;				/* cur chunk */
	FnImplIdx impl;				/* current fn body */
} Compiler;
#endif

#ifdef IMPL
static Expr CompileExpr( Compiler* compiler, Lexer* lexer, Prec min );
static Expr CompileStmt( Compiler* compiler, Lexer* lexer );
static Expr CompileExprAs( Compiler* compiler, Lexer* lexer, Prec min, ExprMode mode );

void CompilerInit( Compiler* compiler, App* app ){
	compiler->logs = &app->logs;
	compiler->interns = &app->interns;
	compiler->consts = &app->consts;
	compiler->locals = &app->locals;
	compiler->insts = &app->insts;
	compiler->chunks = &app->chunks;
	compiler->ifs = &app->ifs;
	compiler->loops = &app->loops;
	compiler->fns = &app->fns;
	compiler->lexer = &app->lexer;

	compiler->nregs = 0;
	compiler->nloops = 0;
	compiler->dst = REG_NONE;
	compiler->chunk = CHUNK_NONE;
	compiler->mode = EM_STMT;
	compiler->impl = FN_IMPL_NONE;
}

static Reg RegAlloc( Compiler* compiler ){
	if( compiler->nregs >= REG_CAP ) Halt( ERR_REGALLOC );
	return ( Reg )compiler->nregs++;
}

static Reg RegResult( Compiler* compiler ){
	if( compiler->dst == REG_NONE ) return RegAlloc( compiler );
	return compiler->dst;
}

// static Reg RegReserve( Compiler* compiler, Reg nregs ){
// 	if( compiler->nregs + nregs > REG_CAP ) Halt( ERR_REGALLOC );
// 	Reg reg = compiler->nregs;
// 	compiler->nregs += nregs;
// 	return reg;
// }

// static void RegEnsure( Compiler* compiler, Reg nregs ){
// 	if( nregs > REG_CAP ) Halt( ERR_REGALLOC );
// 	if( compiler->nregs < nregs ) compiler->nregs = nregs;
// }

static ChunkIdx CompilerPushChunk( Compiler* compiler, CompilerFrame* out ){
	out->reg = compiler->nregs;
	out->dst = compiler->dst;
	out->mode = compiler->mode;
	out->nlocals = compiler->locals->len;
	out->chunk = compiler->chunk; /* prev chunk */
	out->impl = compiler->impl;
	ChunkIdx chunk_idx = ChunkPush( compiler->chunks );
	Chunk* chunk = ChunkGet( compiler->chunks, chunk_idx );
	chunk->start = compiler->insts->len;
	chunk->len = 0;
	chunk->nregs = 0;
	compiler->dst = REG_NONE;
	compiler->mode = EM_STMT;
	compiler->chunk = chunk_idx;
	compiler->nregs = 0;
	compiler->locals->len = 0;
	compiler->impl = FN_IMPL_NONE;
	return chunk_idx;
}

static void CompilerPopChunk( Compiler* compiler, CompilerFrame* in ){
	Chunk* chunk = ChunkGet( compiler->chunks, compiler->chunk );
	chunk->len = compiler->insts->len - chunk->start;
	chunk->nregs = compiler->nregs;
	compiler->nregs = in->reg;
	compiler->dst = in->dst;
	compiler->mode = in->mode;
	compiler->locals->len = in->nlocals;
	compiler->chunk = in->chunk;
	compiler->impl = in->impl;
}

static InstIdx CompilerGetIp( Compiler* compiler ){
	Chunk* chunk = ChunkGet( compiler->chunks, compiler->chunk );
	return compiler->insts->len - chunk->start;
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
	if( expr.type != EXPR_ID ) return expr;
	Local* local = LocalGet( compiler->locals, expr.intern );
	if( !local ) return CompileBadId( compiler, pos, expr.intern );
	return ExprAs( local->expr_type, local->reg );
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
	if( compiler->mode != EM_STMT || !OpIsMutative( op->code ) ) return 0;
	InstABC( compiler->insts, op->code, src->reg, src->reg, 0 ); /* no alloc, self-mutate */
	return 1;
}

static Expr CompileUnary( Compiler* compiler, Lexer* lexer, Tk* tk ){
	Expr src = CompileExprAs( compiler, lexer, PREC_UNARY, EM_EXPR );
	// src = CompileReg( compiler, src, &tk->pos );
	Op* op = OpGetUnary( src.type, tk->type );
	if( !op->code ) return CompileBadUnary( compiler, &src, tk );
	if( CompileUnaryStmt( compiler, op, &src ) ) return ExprVoid( );
	Expr dst = ExprAs( op->type, RegResult( compiler ) );
	InstABC( compiler->insts, op->code, dst.reg, src.reg, 0 );
	return dst;
}

static Expr CompileNum( Compiler* compiler, Tk* tk ){
	Expr dst = ExprAs( EXPR_NUM, RegResult( compiler ) );
	ConstIdx idx = ConstPutNum( compiler->consts, tk->num );
	InstABX( compiler->insts, OP_LOADC, dst.reg, idx );
	return dst;
}

static Expr CompileStr( Compiler* compiler, Tk* tk ){
	Expr dst = ExprAs( EXPR_STR, RegResult( compiler ) );
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
	if( compiler->mode != EM_STMT || !OpIsMutative( op->code ) ) return 0;
	Op* mut = OpGetUnary( src->type, tk->type ); /* use prefix, less vm overhead */
	InstABC( compiler->insts, mut->code, src->reg, src->reg, 0 ); /* no alloc, self-mutate. */
	return 1;
}

static Expr CompilePost( Compiler* compiler, Lexer* lexer, Expr src, Tk* tk ){
	src = CompileReg( compiler, src, &tk->pos );
	Lex( lexer );
	Op* op = OpGetPost( src.type, tk->type );
	if( !op->code ) return CompileBadPost( compiler, &src, tk );
	if( CompilePostStmt( compiler, op, &src, tk ) ) return ExprVoid( );
	Expr dst = ExprAs( op->type, RegResult( compiler ) );
	InstABC( compiler->insts, op->code, dst.reg, src.reg, 0 );
	return dst;
}

static Expr CompileCall( Compiler* compiler, Lexer* lexer, Expr src, Tk* tk ){
	( void )lexer;
	( void )compiler;
	( void )tk;
	if( src.type != EXPR_ID ) return ExprErr( );
	return ExprVoid( );
}

static Expr CompilePostfix( Compiler* compiler, Lexer* lexer, Expr src ){
	for( ;; ){
		Tk tk = lexer->tk; /* copy required */
		Deno deno = DenoGet( PARSEPOS_POST, tk.type );
		switch( deno ){
			default: return src;
			case DENO_POST: src = CompilePost( compiler, lexer, src, &tk ); break;
			case DENO_CALL: src = CompileCall( compiler, lexer, src, &tk ); break;
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
	if( compiler->mode != EM_STMT || !OpIsMutative( op->code ) ) return 0;	
	InstABC( compiler->insts, op->code, lhs->reg, lhs->reg, rhs->reg ); /* no alloc, self-mutate */
	return 1;
}

static Expr CompileBinary( Compiler* compiler, Lexer* lexer, Expr lhs, Prec prec, Tk* tk ){
	Lex( lexer );
	Expr rhs = CompileExprAs( compiler, lexer, prec, EM_EXPR );
	// rhs = CompileReg( compiler, rhs, &tk->pos );
	Op* op = OpGetBinary( lhs.type, rhs.type, tk->type );
	if( !op->code ) return CompileBadBinary( compiler, &lhs, &rhs, tk );
	if( CompileBinaryStmt( compiler, op, &lhs, &rhs ) ) return ExprVoid( );
	Expr dst = ExprAs( op->type, RegResult( compiler ) );
	InstABC( compiler->insts, op->code, dst.reg, lhs.reg, rhs.reg );
	return dst;
}

static Expr CompileInfix( Compiler* compiler, Lexer* lexer, Expr lhs, Prec min ){
	Tk tk = lexer->tk;
	lhs = CompileReg( compiler, lhs, &tk.pos );
	for( ;; ){
		tk = lexer->tk; /* copy */
		if( DenoGet( PARSEPOS_INF, tk.type ) != DENO_INF ) return lhs;
		Prec prec = PrecGet( tk.type );
		if( prec <= min ) return lhs;
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
/* compile an expr into a specificed register. */
// static Expr CompileExprInto( Compiler* compiler, Lexer* lexer, Prec min, Reg new_dst ){
// 	ExprMode old_mode = compiler->mode;
// 	Reg old_dst = compiler->dst;
// 	compiler->mode = EM_EXPR;
// 	compiler->dst = new_dst;
// 	Expr expr = CompileExpr( compiler, lexer, min );
// expr = CompileReg( compiler, expr, &lexer->tk.pos );
// 	if( expr.type != EXPR_ERR && expr.type != EXPR_VOID && expr.reg != new_dst ){
// 		InstABC( compiler->insts, OP_MOV, new_dst, expr.reg, 0 );
// 		expr = ExprAs( expr.type, new_dst );
// 	}
// 	compiler->mode = old_mode;
// 	compiler->dst = old_dst;
// 	return expr;
// }

static void CompileLoopBody( Compiler* compiler, Lexer* lexer ){
	compiler->nloops++;
	u32 old_locals = compiler->locals->len;
	while( lexer->tk.type != TK_END && lexer->tk.type != TK_EOS )
		CompileStmt( compiler, lexer );
	compiler->locals->len = old_locals;
	compiler->nloops--;
}

static Expr CompileLoop( Compiler* compiler, Lexer* lexer ){
	Lex( lexer ); /* eat ;; */
	PatchIdx mark = compiler->loops->len;
	InstIdx enter = CompilerGetIp( compiler );
	CompileLoopBody( compiler, lexer );
	InstJmp( compiler->insts, enter );
	InstIdx exit = CompilerGetIp( compiler );
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
	if( !compiler->nloops ) return CompileBadControl( compiler, &tk.pos, log_type );
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
			CompileStmt( compiler, lexer );
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
	if( lexer->tk.type == TK_ELIF || lexer->tk.type == TK_ELSE ) return 0;
	PatchApply( compiler->ifs, PATCH_BRANCH, branch, CompilerGetIp( compiler ) );
	return 1;
}

static void CompileIfBranch( Compiler* compiler, Lexer* lexer ){
	PatchIdx branch = CompileIfCond( compiler, lexer );
	CompileIfBody( compiler, lexer );
	if( CompileIfLast( compiler, lexer, branch ) ) return;
	InstIdx end = InstJmp( compiler->insts, 0 );
	PatchApply( compiler->ifs, PATCH_BRANCH, branch, CompilerGetIp( compiler ) );
	PatchPush( compiler->ifs, PATCH_END, end );
}

static void CompileElse( Compiler* compiler, Lexer* lexer ){
	if( lexer->tk.type != TK_ELSE ) return;
	Lex( lexer ); /* ?? */
	CompileIfBody( compiler, lexer );
}

static Expr CompileIf( Compiler* compiler, Lexer* lexer ){
	PatchIdx mark = compiler->ifs->len;
	do CompileIfBranch( compiler, lexer ); while( lexer->tk.type == TK_ELIF );
	CompileElse( compiler, lexer );
	CompilerMatch( compiler, lexer, TK_END );
	PatchApply( compiler->ifs, PATCH_END, mark, CompilerGetIp( compiler ) );
	return ExprVoid( );
}

static Expr CompileDecl( Compiler* compiler, Lexer* lexer ){
	if( lexer->peek.type != TK_ASSIGN ) return CompileExprAs( compiler, lexer, PREC_NONE, EM_STMT );
	Tk tk = lexer->tk;
	Lex( lexer ); /* eat id */
	Lex( lexer ); /* eat : */
	Expr rhs = CompileExprAs( compiler, lexer, PREC_NONE, EM_EXPR );
	if( rhs.type == EXPR_ERR ) return rhs;
	LocalPut( compiler->locals, tk.intern, rhs.type, rhs.reg );
	return rhs;
}

static void CompileSkipBody( Compiler* compiler, Lexer* lexer, SrcSpan* out ){
	out->src = lexer->tk.pos.src;
	out->start = lexer->tk.pos.off;
	while( lexer->tk.type != TK_END && lexer->tk.type != TK_EOS ) Lex( lexer );
	out->end = lexer->tk.pos.off;
	CompilerMatch( compiler, lexer, TK_END );
}

static u32 CompileFnArgs( Compiler* compiler, Lexer* lexer, InternIdx* args ){
	u32 nargs = 0;
	while( lexer->tk.type != TK_FNCLOSE && lexer->tk.type != TK_EOS ){
		if( nargs >= UINT8_MAX ) Halt( ERR_REGALLOC );
		InternIdx arg = lexer->tk.intern;
		CompilerMatch( compiler, lexer, TK_ID );
		args[ nargs++ ] = arg;
		if( lexer->tk.type != TK_COMMA ) break;
		Lex( lexer );
	}
	return nargs;
}

static Expr CompileFnDecl( Compiler* compiler, Lexer* lexer ){
	InternIdx name = lexer->tk.intern;
	Lex( lexer ); /* eat id */
	CompilerMatch( compiler, lexer, TK_FNOPEN ); /* <( */
	InternIdx args[ REG_CAP ];
	u32 nargs = CompileFnArgs( compiler, lexer, args );
	CompilerMatch( compiler, lexer, TK_FNCLOSE ); /* )> */
	SrcSpan body;
	CompileSkipBody( compiler, lexer, &body );
	FnArgIdx arg_base = FnArgsPush( compiler->fns, args, ( u8 )nargs );
	FnDeclPush( compiler->fns, name, &body, arg_base, ( u8 )nargs );
	return ExprVoid( );
}

static Expr CompileIdStmt( Compiler* compiler, Lexer* lexer ){
	if( lexer->peek.type == TK_FNOPEN ) return CompileFnDecl( compiler, lexer );
	return CompileDecl( compiler, lexer );
}

static Expr CompileReturn( Compiler* compiler, Lexer* lexer ){
	Tk tk = lexer->tk;
	Lex( lexer ); /* <- */
	if( compiler->impl == FN_IMPL_NONE ){
		Log( compiler->logs, &tk.pos, CMP_BADRET );
		return ExprErr( );
	}
	Expr ret = CompileExprAs( compiler, lexer, PREC_NONE, EM_EXPR );
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
		case TK_ID: return CompileIdStmt( compiler, lexer );
		case TK_EOS: return ExprErr( );
	}
}

ChunkIdx CompilerRun( Compiler* compiler ){
	Lexer* lexer = compiler->lexer;
	CompilerFrame entry;
	ChunkIdx chunk_idx = CompilerPushChunk( compiler, &entry );
	Expr expr = ExprVoid( );
	while( lexer->tk.type != TK_EOS ) expr = CompileStmt( compiler, lexer );
	if( expr.type == EXPR_VOID ) expr = CompileVoid( compiler );
	InstABC( compiler->insts, OP_HALT, expr.reg, 0, 0 );
	CompilerPopChunk( compiler, &entry );
	return chunk_idx;
}
#endif