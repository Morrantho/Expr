#ifdef TYPES
typedef struct Lexer {
	SrcPos pos;
	Srcs* srcs;
	Logs* logs;
	Interns* interns;
	u8* text;
	Tk tk;
	Tk peek;
} Lexer;
#endif

#ifdef IMPL
static void LexScan( Lexer* lexer, Tk* tk );

void LexInit( Lexer* lexer, App* app, SrcIdx src ){
	lexer->srcs = &app->srcs;
	lexer->logs = &app->logs;
	lexer->interns = &app->interns;
	lexer->pos.src = src;
	lexer->pos.off = 0;
	lexer->pos.ln = lexer->pos.col = 1;
	lexer->text = SrcGetText( lexer->srcs, src );
	LexScan( lexer, &lexer->tk );
	LexScan( lexer, &lexer->peek );
}

static u8 LexGetType( u8 ascii ){
	static u8 types[ ] = { X_ASCIIS( X_LEX_TYPE_INIT ) };
	return types[ ascii ];
}

static void LexNext( Lexer* lexer, Tk* tk ){
	( void )tk;
	++lexer->text;
	++lexer->pos.off;
	++lexer->pos.col;
}

/* Snapshots source position */
static void LexSet( Lexer* lexer, Tk* tk, TkType type ){
	tk->type = type;
	tk->pos.ln = lexer->pos.ln;
	tk->pos.col = lexer->pos.col;
}

/* Single chars only. Multi-char ops can call this once. */
static void LexChar( Lexer* lexer, Tk* tk, TkType type ){
	LexSet( lexer, tk, type );
	LexNext( lexer, tk );
}

/* Only called after LexSet or LexChar. Only used for multi-char ops. */
static void LexEat( Lexer* lexer, Tk* tk, TkType type ){
	tk->type = type;
	LexNext( lexer, tk );
}

static void LexLine( Lexer* lexer, Tk* tk ){ /* \n */
	( void )tk;
	++lexer->text;
	++lexer->pos.off;
	++lexer->pos.ln;
	lexer->pos.col = 1;
}

static void LexEos( Lexer* lexer, Tk* tk ){ LexChar( lexer, tk, TK_EOS ); }

static void LexNot( Lexer* lexer, Tk* tk ){ /* ! != */
	LexChar( lexer, tk, TK_NOT );
	if( *lexer->text == '=' ){ LexEat( lexer, tk, TK_NOTEQ ); }
}

static void LexStr( Lexer* lexer, Tk* tk ){ /* "anything in between" */
	SrcPos pos = lexer->pos;
	LexChar( lexer, tk, TK_STR ); /* " */
	u8* start = lexer->text;
	u32 hash = HashStart( HASH_STR );
	while( *lexer->text && *lexer->text != '"' ){
		hash = HashU8( hash, *lexer->text );
		LexNext( lexer, tk );
	}
	if( !*lexer->text ){
		Log( lexer->logs, &pos, LEX_BADSTR );
		LexSet( lexer, tk, TK_ERR );
		return;
	}
	hash = HashEnd( hash );
	u32 len = ( u32 )( lexer->text - start );
	tk->intern = InternPutStr( lexer->interns, start, len, hash );
	LexNext( lexer, tk ); /* " */
}

static void LexComment( Lexer* lexer, Tk* tk ){ /* $ */
	LexNext( lexer, tk );
	for( ; *lexer->text != '\n'; LexNext( lexer, tk ) )
		if( !*lexer->text ) break;
}

static void LexMod( Lexer* lexer, Tk* tk ){ /* % %% %= */
	LexChar( lexer, tk, TK_MOD );
	if( *lexer->text == '%' ){ LexEat( lexer, tk, TK_ROUND ); return; }
	if( *lexer->text == '=' ){ LexEat( lexer, tk, TK_MODEQ ); return; }
}

static void LexLp( Lexer* lexer, Tk* tk ){ LexChar( lexer, tk, TK_LP ); }

static void LexRp( Lexer* lexer, Tk* tk ){ /* ) )> */
	LexChar( lexer, tk, TK_RP );
	if( *lexer->text == '>' ){ LexEat( lexer, tk, TK_FNCLOSE ); return; }
}

static void LexBand( Lexer* lexer, Tk* tk ){ /* & && &= */
	LexChar( lexer, tk, TK_BAND );
	if( *lexer->text == '&' ){ LexEat( lexer, tk, TK_AND ); return; }
	if( *lexer->text == '=' ){ LexEat( lexer, tk, TK_BANDEQ ); return; }
}

static void LexMul( Lexer* lexer, Tk* tk ){ /* * ** *= */
	LexChar( lexer, tk, TK_MUL );
	if( *lexer->text == '*' ){ LexEat( lexer, tk, TK_CEIL ); return; }
	if( *lexer->text == '=' ){ LexEat( lexer, tk, TK_MULEQ ); return; }
}

static void LexAdd( Lexer* lexer, Tk* tk ){ /* + ++ += */
	LexChar( lexer, tk, TK_ADD );
	if( *lexer->text == '+' ){ LexEat( lexer, tk, TK_INC ); return; }
	if( *lexer->text == '=' ){ LexEat( lexer, tk, TK_ADDEQ ); return; }
}

static void LexComma( Lexer* lexer, Tk* tk ){ /* , */
	LexChar( lexer, tk, TK_COMMA );
}

static void LexSub( Lexer* lexer, Tk* tk ){ /* - -- -= */
	LexChar( lexer, tk, TK_SUB );
	if( *lexer->text == '-' ){ LexEat( lexer, tk, TK_DEC ); return; }
	if( *lexer->text == '=' ){ LexEat( lexer, tk, TK_SUBEQ ); return; }
}

static void LexPeriod( Lexer* lexer, Tk* tk ){ /* . */
	LexChar( lexer, tk, TK_MEMBER );
}

static void LexDiv( Lexer* lexer, Tk* tk ){ /* / // /= */
	LexChar( lexer, tk, TK_DIV );
	if( *lexer->text == '/' ){ LexEat( lexer, tk, TK_FLOOR ); return; }
	if( *lexer->text == '=' ){ LexEat( lexer, tk, TK_DIVEQ ); return; }
}

static void LexNum( Lexer* lexer, Tk* tk ){
	LexSet( lexer, tk, TK_NUM );
	f64 n = 0;
	for( ; LexGetType( *lexer->text ) == TK_NUM; LexNext( lexer, tk ) )
		n = n * 10.0 + *lexer->text - '0';
	if( *lexer->text == '.' && LexGetType( lexer->text[ 1 ] ) == TK_NUM ){
		LexNext( lexer, tk );
		for( f64 f = 0.1; LexGetType( *lexer->text ) == TK_NUM; ){
			n += ( lexer->text[ 0 ] - '0' ) * f;
			LexNext( lexer, tk );
			f *= 0.1;
		}
	}
	tk->num = n;
}

static void LexColon( Lexer* lexer, Tk* tk ){ /* : :: */
	LexChar( lexer, tk, TK_ASSIGN );
	if( *lexer->text == ':' ){ LexEat( lexer, tk, TK_END ); return; }
}

static void LexSemi( Lexer* lexer, Tk* tk ){ /* ;; */
	SrcPos pos = lexer->pos;
	LexChar( lexer, tk, TK_EOS );
	if( *lexer->text != ';' ){ Log( lexer->logs, &pos, LEX_BADLOOP ); return; }
	LexEat( lexer, tk, TK_LOOP );
}

static void LexLt( Lexer* lexer, Tk* tk ){ /* < << <<= <= <( <== */
	LexChar( lexer, tk, TK_LT );
	if( *lexer->text == '<' ){
		LexEat( lexer, tk, TK_LSH );
		if( *lexer->text == '=' ){ LexEat( lexer, tk, TK_LSHEQ ); return; }
		return;
	}
	if( *lexer->text == '(' ){
		LexEat( lexer, tk, TK_FNOPEN );
		return;
	}
	if( *lexer->text == '=' ){
		LexEat( lexer, tk, TK_LTE );
		if( *lexer->text == '=' ){ LexEat( lexer, tk, TK_BREAK ); return; }
		return;
	}
}

static void LexEq( Lexer* lexer, Tk* tk ){ /* == ==> */
	SrcPos pos = lexer->pos;
	LexChar( lexer, tk, TK_EOS ); /* Intentional */
	if( *lexer->text != '=' ){ Log( lexer->logs, &pos, LEX_BADASSIGN ); return; }
	LexEat( lexer, tk, TK_CMP );
	if( *lexer->text == '>' ){ LexEat( lexer, tk, TK_CONT ); return; }
}

static void LexGt( Lexer* lexer, Tk* tk ){ /* > >> >>= >= */
	LexChar( lexer, tk, TK_GT );
	if( *lexer->text == '>' ){
		LexEat( lexer, tk, TK_RSH );
		if( *lexer->text == '=' ){ LexEat( lexer, tk, TK_RSHEQ ); return; }
		return;
	}
	if( *lexer->text == '=' ){ LexEat( lexer, tk, TK_GTE ); }
}

static void LexQuestion( Lexer* lexer, Tk* tk ){ /* ?( ?? ??( */
	SrcPos pos = lexer->pos;
	LexChar( lexer, tk, TK_EOS ); /* Intentional */
	if( *lexer->text == '(' ){ LexEat( lexer, tk, TK_IF ); return; } /* ?( */
	if( *lexer->text == '?' ){ /* ?? */
		LexEat( lexer, tk, TK_ELSE );
		if( *lexer->text == '(' ){ LexEat( lexer, tk, TK_ELIF ); return; } /* ??( */
		return;
	}
	Log( lexer->logs, &pos, LEX_BADIF );
}

static void LexAt( Lexer* lexer, Tk* tk ){ LexChar( lexer, tk, TK_RET ); }

static void LexId( Lexer* lexer, Tk* tk ){
	LexSet( lexer, tk, TK_ID );
	u8* start = lexer->text;
	u32 hash = HashStart( HASH_ID );
	for( ;; ){
		TkType type = LexGetType( *lexer->text );
		if( type != TK_ID && type != TK_NUM ) break;
		hash = HashU8( hash, *lexer->text );
		LexNext( lexer, tk );
	}
	hash = HashEnd( hash );
	u32 len = ( u32 )( lexer->text - start );
	tk->intern = InternPutId( lexer->interns, start, len, hash );
}

static void LexBxor( Lexer* lexer, Tk* tk ){ /* ^ ^^ ^= */
	LexChar( lexer, tk, TK_BXOR );
	if( *lexer->text == '^' ){ LexEat( lexer, tk, TK_POW ); return; }
	if( *lexer->text == '=' ){ LexEat( lexer, tk, TK_BXOREQ ); return; }
}

static void LexBor( Lexer* lexer, Tk* tk ){ /* | || |= */
	LexChar( lexer, tk, TK_BOR );
	if( *lexer->text == '|' ){ LexEat( lexer, tk, TK_OR ); return; }
	if( *lexer->text == '=' ){ LexEat( lexer, tk, TK_BOREQ ); return; }
}

static void LexBnot( Lexer* lexer, Tk* tk ){ LexChar( lexer, tk, TK_BNOT ); } /* ~ */

static void LexScan( Lexer* lexer, Tk* out ){
	for( ;; ){
		switch( ( Ascii )*lexer->text ){ X_ASCIIS( X_LEX_CASE ) }
	}
}

void Lex( Lexer* lexer ){
	lexer->tk = lexer->peek;
	LexScan( lexer, &lexer->peek );
}
#endif