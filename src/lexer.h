#ifdef TYPES
#define X_LEX_TYPE_INIT( CHAR, TK, FN, ACTION ) TK_##TK,
#define X_LEX_CASE( CHAR, TK, FN, ACTION ) case ASCII_##CHAR:{ Lex##FN( lexer ); ACTION; }

typedef struct Lexer {
	SrcPos pos;
	Srcs* srcs;
	Logs* logs;
	Interns* interns;
	u8* text;
	Tk tk;
} Lexer;
#endif

#ifdef IMPL
void LexInit( App* app, Lexer* lexer, SrcIdx src ){
	lexer->srcs = &app->srcs;
	lexer->logs = &app->logs;
	lexer->interns = &app->interns;
	lexer->pos.src = src;
	lexer->pos.ln = lexer->pos.col = 1;
	lexer->text = SrcGetText( lexer->srcs, src );
	lexer->tk = ( Tk ){ 0 };
}

static u8 LexGetType( u8 ascii ){
	static u8 types[ ] = { X_ASCIIS( X_LEX_TYPE_INIT ) };
	return types[ ascii ];
}

static void LexNext( Lexer* lexer ){
	++lexer->text;
	++lexer->pos.col;
}

/* Snapshots source position */
static void LexSet( Lexer* lexer, TkType type ){
	lexer->tk.type = type;
	lexer->tk.pos.ln = lexer->pos.ln;
	lexer->tk.pos.col = lexer->pos.col;
}

/* Single chars only. Multi-char ops can call this once. */
static void LexChar( Lexer* lexer, TkType type ){
	LexSet( lexer, type );
	LexNext( lexer );
}

/* Only called after LexSet or LexChar. Only used for multi-char ops. */
static void LexEat( Lexer* lexer, TkType type ){
	lexer->tk.type = type;
	LexNext( lexer );
}

static void LexLine( Lexer* lexer ){ /* \n */
	++lexer->text;
	++lexer->pos.ln;
	lexer->pos.col = 1;
}

static void LexEos( Lexer* lexer ){ LexChar( lexer, TK_EOS ); }

static void LexNot( Lexer* lexer ){ /* ! != */
	LexChar( lexer, TK_NOT );
	if( *lexer->text == '=' ){ LexEat( lexer, TK_NOTEQ ); }
}

static void LexStr( Lexer* lexer ){ /* "anything in between" */
	SrcPos pos = lexer->pos;
	LexChar( lexer, TK_STR ); /* " */
	u8* start = lexer->text;
	u32 hash = HashStart( HASH_STR );
	while( *lexer->text && *lexer->text != '"' ){
		hash = HashU8( hash, *lexer->text );
		LexNext( lexer );
	}
	if( !*lexer->text ){
		Log( lexer->logs, &pos, LEX_BADSTR );
		LexSet( lexer, TK_ERR );
		return;
	}
	hash = HashEnd( hash );
	u32 len = ( u32 )( lexer->text - start );
	lexer->tk.intern = InternPutStr( lexer->interns, start, len, hash );
	LexNext( lexer ); /* " */
}

static void LexComment( Lexer* lexer ){ /* $ */
	LexNext( lexer );
	for( ; *lexer->text != '\n'; LexNext( lexer ) )
		if( !*lexer->text ) break;
}

static void LexMod( Lexer* lexer ){ /* % %% %= */
	LexChar( lexer, TK_MOD );
	if( *lexer->text == '%' ){ LexEat( lexer, TK_ROUND ); return; }
	if( *lexer->text == '=' ){ LexEat( lexer, TK_MODEQ ); return; }
}

static void LexLp( Lexer* lexer ){ LexChar( lexer, TK_LP ); }

static void LexRp( Lexer* lexer ){ /* ) )> */
	LexChar( lexer, TK_RP );
	if( *lexer->text == '>' ){ LexEat( lexer, TK_FNCLOSE ); return; }
}

static void LexBand( Lexer* lexer ){ /* & && &= */
	LexChar( lexer, TK_BAND );
	if( *lexer->text == '&' ){ LexEat( lexer, TK_AND ); return; }
	if( *lexer->text == '=' ){ LexEat( lexer, TK_BANDEQ ); return; }
}

static void LexMul( Lexer* lexer ){ /* * ** *= */
	LexChar( lexer, TK_MUL );
	if( *lexer->text == '*' ){ LexEat( lexer, TK_CEIL ); return; }
	if( *lexer->text == '=' ){ LexEat( lexer, TK_MULEQ ); return; }
}

static void LexAdd( Lexer* lexer ){ /* + ++ += */
	LexChar( lexer, TK_ADD );
	if( *lexer->text == '+' ){ LexEat( lexer, TK_INC ); return; }
	if( *lexer->text == '=' ){ LexEat( lexer, TK_ADDEQ ); return; }
}

static void LexComma( Lexer* lexer ){ /* , */
	LexChar( lexer, TK_COMMA );
}

static void LexSub( Lexer* lexer ){ /* - -- -= */
	LexChar( lexer, TK_SUB );
	if( *lexer->text == '-' ){ LexEat( lexer, TK_DEC ); return; }
	if( *lexer->text == '=' ){ LexEat( lexer, TK_SUBEQ ); return; }
}

static void LexPeriod( Lexer* lexer ){ /* . */
	LexChar( lexer, TK_MEMBER );
}

static void LexDiv( Lexer* lexer ){ /* / // /= */
	LexChar( lexer, TK_DIV );
	if( *lexer->text == '/' ){ LexEat( lexer, TK_FLOOR ); return; }
	if( *lexer->text == '=' ){ LexEat( lexer, TK_DIVEQ ); return; }
}

static void LexNum( Lexer* lexer ){
	LexSet( lexer, TK_NUM );
	f64 n = 0;
	for( ; LexGetType( *lexer->text ) == TK_NUM; LexNext( lexer ) )
		n = n * 10.0 + *lexer->text - '0';
	if( *lexer->text == '.' && LexGetType( lexer->text[ 1 ] ) == TK_NUM ){
		LexNext( lexer );
		for( f64 f = 0.1; LexGetType( *lexer->text ) == TK_NUM; ){
			n += ( lexer->text[ 0 ] - '0' ) * f;
			LexNext( lexer );
			f *= 0.1;
		}
	}
	lexer->tk.num = n;
}

static void LexColon( Lexer* lexer ){ /* : :: */
	LexChar( lexer, TK_ASSIGN );
	if( *lexer->text == ':' ){ LexEat( lexer, TK_END ); return; }
}

static void LexSemi( Lexer* lexer ){ /* ;; */
	SrcPos pos = lexer->pos;
	LexChar( lexer, TK_EOS );
	if( *lexer->text != ';' ){ Log( lexer->logs, &pos, LEX_BADLOOP ); return; }
	LexEat( lexer, TK_LOOP );
}

static void LexLt( Lexer* lexer ){ /* < << <<= <= <( <== */
	LexChar( lexer, TK_LT );
	if( *lexer->text == '<' ){
		LexEat( lexer, TK_LSH );
		if( *lexer->text == '=' ){ LexEat( lexer, TK_LSHEQ ); return; }
		return;
	}
	if( *lexer->text == '(' ){
		LexEat( lexer, TK_FNOPEN );
		return;
	}
	if( *lexer->text == '=' ){
		LexEat( lexer, TK_LTE );
		if( *lexer->text == '=' ){ LexEat( lexer, TK_BREAK ); return; }
		return;
	}
}

static void LexEq( Lexer* lexer ){ /* == ==> */
	SrcPos pos = lexer->pos;
	LexChar( lexer, TK_EOS ); /* Intentional */
	if( *lexer->text != '=' ){ Log( lexer->logs, &pos, LEX_BADASSIGN ); return; }
	LexEat( lexer, TK_ISEQ );
	if( *lexer->text == '>' ){ LexEat( lexer, TK_CONT ); return; }
}

static void LexGt( Lexer* lexer ){ /* > >> >>= >= */
	LexChar( lexer, TK_GT );
	if( *lexer->text == '>' ){
		LexEat( lexer, TK_RSH );
		if( *lexer->text == '=' ){ LexEat( lexer, TK_RSHEQ ); return; }
		return;
	}
	if( *lexer->text == '=' ){ LexEat( lexer, TK_GTE ); }
}

static void LexQuestion( Lexer* lexer ){ /* ?( ?? ??( */
	SrcPos pos = lexer->pos;
	LexChar( lexer, TK_EOS ); /* Intentional */
	if( *lexer->text == '(' ){ LexEat( lexer, TK_IF ); return; } /* ?( */
	if( *lexer->text == '?' ){ /* ?? */
		LexEat( lexer, TK_ELSE );
		if( *lexer->text == '(' ){ LexEat( lexer, TK_ELIF ); return; } /* ??( */
		return;
	}
	Log( lexer->logs, &pos, LEX_BADIF );
}

static void LexAt( Lexer* lexer ){ LexChar( lexer, TK_RET ); }

static void LexId( Lexer* lexer ){
	LexSet( lexer, TK_ID );
	u8* start = lexer->text;
	u32 hash = HashStart( HASH_ID );
	for( ;; ){
		TkType type = LexGetType( *lexer->text );
		if( type != TK_ID && type != TK_NUM ) break;
		hash = HashU8( hash, *lexer->text );
		LexNext( lexer );
	}
	hash = HashEnd( hash );
	u32 len = ( u32 )( lexer->text - start );
	lexer->tk.intern = InternPutId( lexer->interns, start, len, hash );
}

static void LexBxor( Lexer* lexer ){ /* ^ ^^ ^= */
	LexChar( lexer, TK_BXOR );
	if( *lexer->text == '^' ){ LexEat( lexer, TK_POW ); return; }
	if( *lexer->text == '=' ){ LexEat( lexer, TK_BXOREQ ); return; }
}

static void LexBor( Lexer* lexer ){ /* | || |= */
	LexChar( lexer, TK_BOR );
	if( *lexer->text == '|' ){ LexEat( lexer, TK_OR ); return; }
	if( *lexer->text == '=' ){ LexEat( lexer, TK_BOREQ ); return; }
}

static void LexBnot( Lexer* lexer ){ LexChar( lexer, TK_BNOT ); } /* ~ */

void Lex( Lexer* lexer ){
	for( ;; ){
		switch( ( Ascii )*lexer->text ){ X_ASCIIS( X_LEX_CASE ) }
	}
}
#endif