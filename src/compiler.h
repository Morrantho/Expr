#ifdef TYPES
typedef u32 Reg;
#define REG_NONE UINT32_MAX

typedef struct CompilerScope {
	u32 nlocals;
} CompilerScope;

typedef struct CompilerFrame {	/* register span */
	Reg reg;					/* base reg */
	ChunkIdx chunk;				/* store previous chunk */
	u32 nlocals;
} CompilerFrame;

typedef struct Compiler {
	Logs* logs;
	Interns* interns;
	Consts* consts;
	Locals* locals;
	Insts* insts;
	Chunks* chunks;
	Patches *ifs, *loops;
	Lexer* lexer;

	Reg nregs;
	ChunkIdx chunk;				/* cur chunk */
	u32 nloops;					/* continue + break need this for context */
} Compiler;
#endif

#ifdef IMPL
static Expr CompileExpr( Compiler* compiler, Lexer* lexer, Prec min );
static Expr CompileStmt( Compiler* compiler, Lexer* lexer );

void CompilerInit( Compiler* compiler, App* app ){
	compiler->logs = &app->logs;
	compiler->interns = &app->interns;
	compiler->consts = &app->consts;
	compiler->locals = &app->locals;
	compiler->insts = &app->insts;
	compiler->chunks = &app->chunks;
	compiler->ifs = &app->ifs;
	compiler->loops = &app->loops;
	compiler->lexer = &app->lexer;
	compiler->nregs = 0;
	compiler->chunk = CHUNK_NONE;
	compiler->nloops = 0;
}

static Reg RegAlloc( Compiler* compiler ){
	if( compiler->nregs >= REG_CAP ){ Halt( ERR_REGALLOC ); }
	return ( Reg )compiler->nregs++;
}

static CompilerScope CompilerPushScope( Compiler* compiler ){
	return ( CompilerScope ){ .nlocals = compiler->locals->len };
}

static void CompilerPopScope( Compiler* compiler, CompilerScope* in ){
	compiler->locals->len = in->nlocals;
}

static InstIdx CompilerGetIp( Compiler* compiler ){
	Chunk* chunk = ChunkGet( compiler->chunks, compiler->chunk );
	return compiler->insts->len - chunk->start;
}

static ChunkIdx CompilerPushChunk( Compiler* compiler, CompilerFrame* out ){
	out->reg = compiler->nregs;
	out->nlocals = compiler->locals->len;
	out->chunk = compiler->chunk; /* prev chunk */
	ChunkIdx chunk_idx = ChunkPush( compiler->chunks );
	Chunk* chunk = ChunkGet( compiler->chunks, chunk_idx );
	chunk->start = compiler->insts->len;
	chunk->len = 0;
	chunk->nregs = 0;
	compiler->chunk = chunk_idx;
	compiler->nregs = 0;
	compiler->locals->len = 0;
	return chunk_idx;
}

static void CompilerPopChunk( Compiler* compiler, CompilerFrame* in ){
	Chunk* chunk = ChunkGet( compiler->chunks, compiler->chunk );
	chunk->len = compiler->insts->len - chunk->start;
	chunk->nregs = compiler->nregs;
	compiler->nregs = in->reg;
	compiler->locals->len = in->nlocals;
	compiler->chunk = in->chunk;
}

static void CompilerMatch( Compiler* compiler, Lexer* lexer, TkType expected ){
	Tk* tk = &lexer->tk;
	if( tk->type != expected ){
		Log( compiler->logs, &tk->pos, PARSE_EXPECT, TkGetName( expected ), TkGetName( tk->type ) );
	}
	Lex( lexer );
}

static Expr CompileVoid( Compiler* compiler ){
	Expr expr = ExprGen( EXPR_VOID, RegAlloc( compiler ) );
	InstABX( compiler->insts, OP_LOADC, expr.reg, CONST_VOID );
	return expr;
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

static Expr CompileUnary( Compiler* compiler, Lexer* lexer, Tk* tk ){
	Expr src = CompileExpr( compiler, lexer, PREC_UNARY );
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
	return ExprErr( );
}

static Expr CompileId( Compiler* compiler, Tk* tk ){
	Local* local = LocalGet( compiler->locals, tk->intern );
	if( !local ) return CompileBadId( compiler, tk );
	return ExprGen( local->expr_type, local->reg );
}

static Expr CompilePrefix( Compiler* compiler, Lexer* lexer ){
	Tk tk = lexer->tk; /* copy */
	Deno deno = DenoGet( PARSEPOS_PRE, tk.type );
	Lex( lexer );
	switch( deno ){
		default: return CompileBadPrefix( compiler, deno, &tk );
		case DENO_NOPPRE: return CompileExpr( compiler, lexer, PREC_UNARY );
		case DENO_GRP: return CompileGroup( compiler, lexer );
		case DENO_PRE: return CompileUnary( compiler, lexer, &tk );
		case DENO_NUM: return CompileNum( compiler, &tk );
		case DENO_STR: return CompileStr( compiler, &tk );
		case DENO_ID: return CompileId( compiler, &tk );
	}
}

static Expr CompileBadPost( Compiler* compiler, Expr* src, Tk* tk ){
	u8* src_name = ExprGetName( src->type );
	u8* post_name = TkGetName( tk->type );
	Log( compiler->logs, &tk->pos, CMP_BADPOST, post_name, src_name );
	return ExprErr( );
}

static Expr CompilePost( Compiler* compiler, Lexer* lexer, Expr src, Tk* tk ){
	Lex( lexer );
	Op* op = OpGetPost( src.type, tk->type );
	if( !op->code ) return CompileBadPost( compiler, &src, tk );
	Expr dst = ExprGen( op->type, RegAlloc( compiler ) );
	InstABC( compiler->insts, op->code, dst.reg, src.reg, 0 );
	return dst;
}

static Expr CompilePostfix( Compiler* compiler, Lexer* lexer, Expr src ){
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
	u8* bin_name = TkGetName( tk->type );
	Log( c->logs, &tk->pos, CMP_BADBINARY, bin_name, lhs_type, rhs_type );
	return ExprErr( );
}

static Expr CompileBinary( Compiler* compiler, Lexer* lexer, Expr lhs, Prec prec, Tk* tk ){
	Lex( lexer );
	Expr rhs = CompileExpr( compiler, lexer, prec );
	Op* op = OpGetBinary( lhs.type, rhs.type, tk->type );
	if( !op->code ) return CompileBadBinary( compiler, &lhs, &rhs, tk );
	Expr dst = ExprGen( op->type, RegAlloc( compiler ) );
	InstABC( compiler->insts, op->code, dst.reg, lhs.reg, rhs.reg );
	return dst;
}

static Expr CompileInfix( Compiler* compiler, Lexer* lexer, Expr lhs, Prec min ){
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

static Expr CompileExpr( Compiler* compiler, Lexer* lexer, Prec min ){
	Expr expr = CompilePrefix( compiler, lexer );
	expr = CompilePostfix( compiler, lexer, expr );
	expr = CompileInfix( compiler, lexer, expr, min );
	return expr;
}

static void CompileLoopBody( Compiler* compiler, Lexer* lexer ){
	compiler->nloops++;
	CompilerScope scope = CompilerPushScope( compiler );
	while( lexer->tk.type != TK_END && lexer->tk.type != TK_EOS )
		CompileStmt( compiler, lexer );
	CompilerPopScope( compiler, &scope );
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
	CompilerScope scope = CompilerPushScope( compiler );
	while( lexer->tk.type != TK_END 
		&& lexer->tk.type != TK_ELIF
		&& lexer->tk.type != TK_ELSE
		&& lexer->tk.type != TK_EOS )
			CompileStmt( compiler, lexer );
	CompilerPopScope( compiler, &scope );
}

static PatchIdx CompileIfCond( Compiler* compiler, Lexer* lexer ){
	PatchIdx miss = compiler->ifs->len;
	Lex( lexer ); /* eat ?( or ??( */
	Expr cond = CompileExpr( compiler, lexer, PREC_NONE );
	CompilerMatch( compiler, lexer, TK_RP ); /* match ) */
	PatchPush( compiler->ifs, PATCH_MISS, InstJz( compiler->insts, cond.reg ) );
	return miss;
}

static u8 CompileIfLast( Compiler* compiler, Lexer* lexer, PatchIdx miss ){
	if( lexer->tk.type == TK_ELIF || lexer->tk.type == TK_ELSE ) return 0;
	PatchApply( compiler->ifs, PATCH_MISS, miss, CompilerGetIp( compiler ) );
	return 1;
}

static void CompileIfBranch( Compiler* compiler, Lexer* lexer ){
	PatchIdx miss = CompileIfCond( compiler, lexer );
	CompileIfBody( compiler, lexer );
	if( CompileIfLast( compiler, lexer, miss ) ) return;
	InstIdx end = InstJmp( compiler->insts, 0 );
	PatchApply( compiler->ifs, PATCH_MISS, miss, CompilerGetIp( compiler ) );
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
	if( lexer->peek.type != TK_ASSIGN ) return CompileExpr( compiler, lexer, PREC_NONE );
	Tk tk = lexer->tk;
	Lex( lexer ); /* eat id */
	Lex( lexer ); /* eat : */
	Expr rhs = CompileExpr( compiler, lexer, PREC_NONE );
	if( rhs.type == EXPR_ERR ) return rhs;
	LocalPut( compiler->locals, tk.intern, rhs.type, rhs.reg );
	return rhs;
}

static Expr CompileStmt( Compiler* compiler, Lexer* lexer ){
	switch( lexer->tk.type ){
		default: return CompileExpr( compiler, lexer, PREC_NONE );
		case TK_LOOP: return CompileLoop( compiler, lexer );
		case TK_BREAK: return CompileBreak( compiler, lexer );
		case TK_CONT: return CompileContinue( compiler, lexer );
		case TK_IF: return CompileIf( compiler, lexer );
		case TK_ID: return CompileDecl( compiler, lexer );
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