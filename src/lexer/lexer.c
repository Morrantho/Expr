#include "lexer.h"

void LexInit( Lexer* lexer, u8* text ){
	lexer->text = text;
	lexer->ln = lexer->col = 1;
}

void LexReset( Lexer* lexer, u8* text ){
	lexer->text = text;
	lexer->ln = lexer->col = 1;
}

static u8 LexType( TkType type ){
	static u8 types[ ] = { X_LEX_TYPES( X_LEX_TYPE_INIT ) };
	return types[ type ];
}

static void LexNext( Lexer* lexer ){
	++lexer->text;
	++lexer->col;
}

static void LexEat( Lexer* lexer, TkType type ){
	LexNext( lexer );
	lexer->tk.type = type;
}

static void LexLine( Lexer* lexer ){
	lexer->col = 1;
	++lexer->ln;
}

static void LexEos( Lexer* lexer ){ LexEat( lexer, TK_EOS ); }

static void LexNot( Lexer* lexer ){ /* ! != */
	LexEat( lexer, TK_NOT );
	if( *lexer->text == '=' ){ LexEat( lexer, TK_NOTEQ ); }
}

static void LexComment( Lexer* lexer ){ /* $ */
	LexNext( lexer );
	for( ; *lexer->text != '\n'; LexNext( lexer ) )
		if( !*lexer->text ) break;
}

static void LexMod( Lexer* lexer ){ /* % %% %= */
	LexEat( lexer, TK_MOD );
	if( *lexer->text == '%' ){ LexEat( lexer, TK_ROUND ); return; }
	if( *lexer->text == '=' ){ LexEat( lexer, TK_MODEQ ); return; }
}

static void LexLp( Lexer* lexer ){ LexEat( lexer, TK_LP ); }
static void LexRp( Lexer* lexer ){ LexEat( lexer, TK_RP ); }

static void LexBand( Lexer* lexer ){ /* & && &= */
	LexEat( lexer, TK_BAND );
	if( *lexer->text == '&' ){ LexEat( lexer, TK_AND ); return; }
	if( *lexer->text == '=' ){ LexEat( lexer, TK_BANDEQ ); return; }
}

static void LexMul( Lexer* lexer ){ /* * ** *= */
	LexEat( lexer, TK_MUL );
	if( *lexer->text == '*' ){ LexEat( lexer, TK_CEIL ); return; }
	if( *lexer->text == '=' ){ LexEat( lexer, TK_MULEQ ); return; }
}

static void LexAdd( Lexer* lexer ){ /* + ++ += */
	LexEat( lexer, TK_ADD );
	if( *lexer->text == '+' ){ LexEat( lexer, TK_INC ); return; }
	if( *lexer->text == '=' ){ LexEat( lexer, TK_ADDEQ ); return; }
}

static void LexSub( Lexer* lexer ){ /* - -- -= */
	LexEat( lexer, TK_SUB );
	if( *lexer->text == '-' ){ LexEat( lexer, TK_DEC ); return; }
	if( *lexer->text == '=' ){ LexEat( lexer, TK_SUBEQ ); return; }
}

static void LexDiv( Lexer* lexer ){ /* / // /= */
	LexEat( lexer, TK_DIV );
	if( *lexer->text == '/' ){ LexEat( lexer, TK_FLOOR ); return; }
	if( *lexer->text == '=' ){ LexEat( lexer, TK_DIVEQ ); return; }
}

static void LexNum( Lexer* lexer ){
	f64 n = 0;
	for( ; LexType( *lexer->text ) == TK_NUM; ++lexer->text )
		n = n * 10.0 + *lexer->text - '0';
	if( *lexer->text == '.' && LexType( lexer->text[ 1 ] ) == TK_NUM ){
		++lexer->text;
		for( f64 f = 0.1; LexType( *lexer->text ) == TK_NUM; ++lexer->text, f *= 0.1 )
			n += ( lexer->text[ 0 ] - '0' ) * f;
	}
	lexer->tk.num = n;
	lexer->tk.type = TK_NUM;
}

static void LexLt( Lexer* lexer ){ /* < << <<= <= <== */
	LexEat( lexer, TK_LT );
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
	LexEat( lexer, TK_EOS ); /* Intentional */
	if( *lexer->text != '=' ){ Throw( ERR_LEXASSIGN ); return; }
	LexEat( lexer, TK_ISEQ );
	if( *lexer->text == '>' ){ LexEat( lexer, TK_CONT ); return; }
}

static void LexGt( Lexer* lexer ){
	LexEat( lexer, TK_GT );
	if( *lexer->text == '>' ){
		LexEat( lexer, TK_RSH );
		if( *lexer->text == '=' ){ LexEat( lexer, TK_RSHEQ ); return; }
		return;
	}
	if( *lexer->text == '=' ){ LexEat( lexer, TK_GTE ); }
}

static void LexBxor( Lexer* lexer ){ /* ^ ^^ ^= */
	LexEat( lexer, TK_BXOR );
	if( *lexer->text == '^' ){ LexEat( lexer, TK_POW ); return; }
	if( *lexer->text == '=' ){ LexEat( lexer, TK_BXOREQ ); return; }
}

static void LexBor( Lexer* lexer ){ /* | || |= */
	LexEat( lexer, TK_BOR );
	if( *lexer->text == '|' ){ LexEat( lexer, TK_OR ); return; }
	if( *lexer->text == '=' ){ LexEat( lexer, TK_BOREQ ); return; }
}

static void LexBnot( Lexer* lexer ){ LexEat( lexer, TK_BNOT ); }

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