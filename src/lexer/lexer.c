#include "lexer.h"

void LexInit( Lexer* lexer, Src* src ){
	lexer->src = src;
	lexer->text = src->text;
	lexer->src->ln = lexer->src->col = 1;
}

void LexReset( Lexer* lexer, u8* text ){
	lexer->text = text;
	lexer->src->col = 1;
}

static u8 LexType( TkType type ){
	static u8 types[ ] = { X_LEX_TYPES( X_LEX_TYPE_INIT ) };
	return types[ type ];
}

static void LexNext( Lexer* lexer ){
	++lexer->text;
	++lexer->src->col;
}

/* Snapshots source position */
static void LexSet( Lexer* lexer, TkType type ){
	lexer->tk.type = type;
	lexer->tk.ln = lexer->src->ln;
	lexer->tk.col = lexer->src->col;
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

static void LexLine( Lexer* lexer ){
	++lexer->text;
	++lexer->src->ln;
	lexer->src->col = 1;
}

static void LexEos( Lexer* lexer ){ LexChar( lexer, TK_EOS ); }

static void LexNot( Lexer* lexer ){ /* ! != */
	LexChar( lexer, TK_ADD );
	if( *lexer->text == '=' ){ LexEat( lexer, TK_NOTEQ ); }
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
static void LexRp( Lexer* lexer ){ LexChar( lexer, TK_RP ); }

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

static void LexSub( Lexer* lexer ){ /* - -- -= */
	LexChar( lexer, TK_SUB );
	if( *lexer->text == '-' ){ LexEat( lexer, TK_DEC ); return; }
	if( *lexer->text == '=' ){ LexEat( lexer, TK_SUBEQ ); return; }
}

static void LexDiv( Lexer* lexer ){ /* / // /= */
	LexChar( lexer, TK_DIV );
	if( *lexer->text == '/' ){ LexEat( lexer, TK_FLOOR ); return; }
	if( *lexer->text == '=' ){ LexEat( lexer, TK_DIVEQ ); return; }
}

static void LexNum( Lexer* lexer ){
	LexSet( lexer, TK_NUM );
	f64 n = 0;
	for( ; LexType( *lexer->text ) == TK_NUM; LexNext( lexer ) )
		n = n * 10.0 + *lexer->text - '0';
	if( *lexer->text == '.' && LexType( lexer->text[ 1 ] ) == TK_NUM ){
		LexNext( lexer );
		for( f64 f = 0.1; LexType( *lexer->text ) == TK_NUM; ){
			n += ( lexer->text[ 0 ] - '0' ) * f;
			LexNext( lexer );
			f *= 0.1;
		}
	}
	lexer->tk.num = n;
}

static void LexLt( Lexer* lexer ){ /* < << <<= <= <== */
	LexChar( lexer, TK_LT );
	if( *lexer->text == '<' ){
		LexEat( lexer, TK_LSH );
		if( *lexer->text == '=' ){ LexEat( lexer, TK_LSHEQ ); return; }
		return;
	}
	if( *lexer->text == '=' ){
		LexEat( lexer, TK_LTE );
		if( *lexer->text == '=' ){ LexEat( lexer, TK_BREAK ); return; }
		return;
	}
}

static void LexEq( Lexer* lexer ){ /* == ==> */
	LexChar( lexer, TK_EOS ); /* Intentional */
	if( *lexer->text != '=' ){ Log( lexer->src, LEX_BADASSIGN ); return; }
	LexEat( lexer, TK_ISEQ );
	if( *lexer->text == '>' ){ LexEat( lexer, TK_CONT ); return; }
}

static void LexGt( Lexer* lexer ){
	LexChar( lexer, TK_GT );
	if( *lexer->text == '>' ){
		LexEat( lexer, TK_RSH );
		if( *lexer->text == '=' ){ LexEat( lexer, TK_RSHEQ ); return; }
		return;
	}
	if( *lexer->text == '=' ){ LexEat( lexer, TK_GTE ); }
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

static void LexBnot( Lexer* lexer ){ LexChar( lexer, TK_BNOT ); }

void Lex( Lexer* lexer ){
	#ifdef COMPUTED_GOTO
	static void* labels[ ] = { X_ASCIIS( X_LEX_LABEL_INIT ) };
	for( ;; ){
		goto *labels[ *lexer->text ]; X_LEX_LABELS( X_LEX_LABEL )
	}
	#else
	for( ;; ){
		switch( ( Ascii )*lexer->text ){ X_ASCIIS( X_LEX_CASE ) }
	}
	#endif
}