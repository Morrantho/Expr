#ifdef TYPES
typedef u32 Reg;

typedef struct CompilerScope {
	u32 nlocals;
} CompilerScope;

typedef struct CompilerFrame {	/* register span */
	Reg reg;					/* base reg */
	u8 nlocals;
	ChunkIdx chunk;				/* store previous chunk */
} CompilerFrame;

typedef struct Compiler {
	Logs* logs;
	Interns* interns;
	Consts* consts;
	Locals* locals;
	Insts* insts;
	Chunks* chunks;
	Lexer* lexer;

	Reg nregs;
	ChunkIdx chunk;				/* cur chunk */
} Compiler;
#endif

#ifdef IMPL
static Expr CompileExpr( Compiler* compiler, Lexer* lexer, Prec min );
static Expr CompileStmt( Compiler* compiler, Lexer* lexer );

void CompilerInit( App* app, Compiler* compiler ){
	compiler->logs = &app->logs;
	compiler->interns = &app->interns;
	compiler->consts = &app->consts;
	compiler->locals = &app->locals;
	compiler->insts = &app->insts;
	compiler->chunks = &app->chunks;
	compiler->lexer = &app->lexer;
	compiler->nregs = 0;
	compiler->chunk = CHUNK_NONE;
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

static ChunkIdx CompilerPushFrame( Compiler* compiler, CompilerFrame* out ){
	out->reg = compiler->nregs;
	out->nlocals = ( u8 )compiler->locals->len;
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

static void CompilerPopFrame( Compiler* compiler, CompilerFrame* in ){
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
		Log( compiler->logs, &tk->pos, PARSE_EXPECT, expected, tk->type );
	}
	Lex( lexer );
}

static Expr CompileBadPrefix( Compiler* compiler, Deno deno, Tk* tk ){
	u8 *deno_name = DenoGetName( deno );
	u8 *tk_name = TkGetName( tk->type );
	Log( compiler->logs, &tk->pos, PARSE_BADPRE, deno_name, tk_name );
	return ExprGen( EXPR_ERR, 0 );
}

static Expr CompileGroup( Compiler* compiler, Lexer* lexer ){
	Expr src = CompileExpr( compiler, lexer, PREC_NONE );
	CompilerMatch( compiler, lexer, TK_RP );
	return src;
}

static Expr CompileBadUnary( Compiler* compiler, Expr* expr, Tk* tk ){
	u8* expr_name = ExprGetName( expr->type );
	u8* unary_name = OpGetUnaryName( tk->type );
	Log( compiler->logs, &tk->pos, CMP_BADUNARY, unary_name, expr_name );
	return ExprGen( EXPR_ERR, 0 );
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
	return ExprGen( EXPR_ERR, UINT32_MAX );
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

static Expr CompileBlock( Compiler* compiler, Lexer* lexer, TkType end1, TkType end2 ){
	Expr last = ExprGen( EXPR_ERR, UINT32_MAX );
	CompilerScope scope = CompilerPushScope( compiler );
	while( lexer->tk.type != end1 && lexer->tk.type != end2 && lexer->tk.type != TK_EOS )
		last = CompileStmt( compiler, lexer );
	CompilerPopScope( compiler, &scope );
	if( lexer->tk.type == end1 ){ Lex( lexer ); return last; }
	if( lexer->tk.type == end2 && end2 != TK_EOS ){ return last; }
	CompilerMatch( compiler, lexer, end1 ); /* Report missing end */
	return last;
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
	u8* bin_name = OpGetBinaryName( tk->type );
	Log( c->logs, &tk->pos, CMP_BADBINARY, bin_name, lhs_type, rhs_type );
	return ExprGen( EXPR_ERR, UINT32_MAX );
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

static Expr CompileIf( Compiler* compiler, Lexer* lexer ){
	Lex( lexer ); /* eat ?( */
	Expr cond = CompileExpr( compiler, lexer, PREC_NONE );
	CompilerMatch( compiler, lexer, TK_RP ); /* ) */
	InstIdx jz = InstJz( compiler->insts, cond.reg ); /* backpatch later */
	Expr then = CompileBlock( compiler, lexer, TK_END, TK_ELSE ); /* :: ?? */
	if( lexer->tk.type != TK_ELSE ){
		InstPatchBX( compiler->insts, jz, CompilerGetIp( compiler ) ); /* backpatch jz */
		return then;
	}
	Expr dst = ExprGen( then.type, RegAlloc( compiler ) ); /* join register */
	InstMov( compiler->insts, dst.reg, then.reg ); /* mov jmp target to dst */
	InstIdx jmp = InstJmp( compiler->insts ); /* backpatch later */
	InstPatchBX( compiler->insts, jz, CompilerGetIp( compiler ) ); /* backpatch jz */
	Lex( lexer ); /* eat ?? */
	Expr els = CompileBlock( compiler, lexer, TK_END, TK_EOS ); /* :: EOS */
	InstMov( compiler->insts, dst.reg, els.reg );
	InstPatchBX( compiler->insts, jmp, CompilerGetIp( compiler ) ); /* backpatch jmp */
	return dst;
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
		case TK_IF: return CompileIf( compiler, lexer );
		case TK_ID: return CompileDecl( compiler, lexer );
		case TK_EOS: return ExprGen( EXPR_ERR, UINT32_MAX );
	}
}

ChunkIdx CompilerRun( Compiler* compiler ){
	Lexer* lexer = compiler->lexer;
	CompilerFrame entry;
	ChunkIdx chunk_idx = CompilerPushFrame( compiler, &entry );
	Expr expr = ExprGen( EXPR_ERR, UINT32_MAX );
	while( lexer->tk.type != TK_EOS ) expr = CompileStmt( compiler, lexer );
	InstABC( compiler->insts, OP_HALT, expr.reg, 0, 0 );
	CompilerPopFrame( compiler, &entry );
	return chunk_idx;
}
#endif