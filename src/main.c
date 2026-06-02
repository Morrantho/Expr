#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define REPL_MAX 255
#define REG_MAX 255
#define BC_MAX 1023

#define X_TKS( X )\
	/* NAME    PREC    ASSOC  PREFIX  INFIX  POSTFIX */\
	X( EOS,    NONE,   NONE,  NOP,    ERR,   ERR  ) /* \0  */\
	X( NOT,    UNARY,  RIGHT, PRE,    ERR,   ERR  ) /* !   */\
	X( NOTEQ,  EQUAL,  LEFT,  ERR,    INF,   ERR  ) /* !=  */\
	X( MOD,    FACTOR, LEFT,  ERR,    INF,   ERR  ) /* %   */\
	X( ROUND,  UNARY,  RIGHT, PRE,    ERR,   ERR  ) /* %%  */\
	X( MODEQ,  ASSIGN, RIGHT, ERR,    INF,   ERR  ) /* %=  */\
	X( BAND,   BAND,   LEFT,  ERR,    INF,   ERR  ) /* &   */\
	X( AND,    AND,    LEFT,  ERR,    INF,   ERR  ) /* &&  */\
	X( BANDEQ, ASSIGN, RIGHT, ERR,    INF,   ERR  ) /* &=  */\
	X( LP,     NONE,   NONE,  GRP,    ERR,   ERR  ) /* (   */\
	X( RP,     NONE,   NONE,  ERR,    ERR,   ERR  ) /* )   */\
	X( MUL,    FACTOR, LEFT,  ERR,    INF,   ERR  ) /* *   */\
	X( CEIL,   UNARY,  RIGHT, PRE,    ERR,   ERR  ) /* **  */\
	X( MULEQ,  ASSIGN, RIGHT, ERR,    INF,   ERR  ) /* *=  */\
	X( ADD,    TERM,   LEFT,  NOPPRE, INF,   ERR  ) /* +   */\
	X( INC,    UNARY,  LEFT,  PRE,    ERR,   POST ) /* ++  */\
	X( ADDEQ,  ASSIGN, RIGHT, ERR,    INF,   ERR  ) /* +=  */\
	X( SUB,    TERM,   LEFT,  PRE,    INF,   ERR  ) /* -   */\
	X( DEC,    UNARY,  LEFT,  PRE,    ERR,   POST ) /* --  */\
	X( SUBEQ,  ASSIGN, RIGHT, ERR,    INF,   ERR  ) /* -=  */\
	X( DIV,    FACTOR, LEFT,  ERR,    INF,   ERR  ) /* /   */\
	X( FLOOR,  UNARY,  RIGHT, PRE,    ERR,   ERR  ) /* //  */\
	X( DIVEQ,  ASSIGN, RIGHT, ERR,    INF,   ERR  ) /* /=  */\
	X( NUM,    NONE,   NONE,  NUM,    ERR,   ERR  ) /* 0-9 */\
	X( LT,     REL,    LEFT,  ERR,    INF,   ERR  ) /* <   */\
	X( LSH,    SHIFT,  LEFT,  ERR,    INF,   ERR  ) /* <<  */\
	X( LTE,    REL,    LEFT,  ERR,    INF,   ERR  ) /* <=  */\
	X( LSHEQ,  ASSIGN, RIGHT, ERR,    INF,   ERR  ) /* <<= */\
	X( BREAK,  NONE,   NONE,  ERR,    ERR,   ERR  ) /* <== */\
	X( ISEQ,   EQUAL,  LEFT,  ERR,    INF,   ERR  ) /* ==  */\
	X( CONT,   NONE,   NONE,  ERR,    ERR,   ERR  ) /* ==> */\
	X( GT,     REL,    LEFT,  ERR,    INF,   ERR  ) /* >   */\
	X( RSH,    SHIFT,  LEFT,  ERR,    INF,   ERR  ) /* >>  */\
	X( GTE,    REL,    LEFT,  ERR,    INF,   ERR  ) /* >=  */\
	X( RSHEQ,  ASSIGN, RIGHT, ERR,    INF,   ERR  ) /* >>= */\

#define X_PRECS( X )\
	X( NONE )   /*  */\
	X( COMMA )  /* , */\
	X( ASSIGN ) /* = += -= *= /= %= <<= >>= &= |= */\
	X( TERN )   /* ?: */\
	X( OR )     /* || */\
	X( AND )	/* && */\
	X( BOR )    /* | */\
	X( BXOR )   /* ^ */\
	X( BAND )   /* & */\
	X( EQUAL )  /* == != */\
	X( REL )    /* < <= > >= */\
	X( SHIFT )  /* << >> */\
	X( TERM )   /* + - */\
	X( FACTOR ) /* * / */\
	X( UNARY )  /* + - ! ~ * // ** %% */\
	X( POW )    /* ^^ */\
	X( POST )   /* [ ] ( ) . ++ -- */

#define X_DENOS( X )\
	X( ERR )\
	X( NOP )\
	X( NOPPRE )\
	X( GRP )\
	X( PRE )\
	X( POST )\
	X( INF )\
	X( NUM )	

#define X_OPS_NOEMIT( X )\
	X( NOP,      NO_TK, NO_POS )\
	X( HALT,     NO_TK, NO_POS )\
	X( LOADC,    NO_TK, NO_POS )\
	X( BREAK,    NO_TK, NO_POS )\
	X( CONT,     NO_TK, NO_POS )

#define X_OPS_EMIT( X ) /* TK = Derives from */\
	/* OP       TK      POS */\
	X( NOT,     NOT,    PRE  ) /* !a      */\
	X( NOTEQ,   NOTEQ,  INF  ) /* a != b  */\
	X( MOD,     MOD,    INF  ) /* a % b   */\
	X( ROUND,   ROUND,  PRE  ) /* %%a     */\
	X( MODEQ,   MODEQ,  INF  ) /* a %= b  */\
	X( BAND,    BAND,   INF  ) /* a & b   */\
	X( AND,     AND,    INF  ) /* a && b  */\
	X( BANDEQ,  BANDEQ, INF  ) /* a &= b  */\
	X( MUL,     MUL,    INF  ) /* a * b   */\
	X( CEIL,    CEIL,   PRE  ) /* **a     */\
	X( MULEQ,   MULEQ,  INF  ) /* a *= b  */\
	X( ADD,     ADD,    INF  ) /* a + b   */\
	X( PREINC,  INC,    PRE  ) /* ++a     */\
	X( POSTINC, INC,    POST ) /* a++     */\
	X( ADDEQ,   ADDEQ,  INF  ) /* a += b  */\
	X( SUB,     SUB,    INF  ) /* a - b   */\
	X( NEG,     SUB,    PRE  ) /* -a      */\
	X( PREDEC,  DEC,    PRE  ) /* --a     */\
	X( POSTDEC, DEC,    POST ) /* a--     */\
	X( SUBEQ,   SUBEQ,  INF  ) /* a -= b  */\
	X( DIV,     DIV,    INF  ) /* a / b   */\
	X( FLOOR,   FLOOR,  PRE  ) /* //a     */\
	X( DIVEQ,   DIVEQ,  INF  ) /* a /= b  */\
	X( LT,      LT,     INF  ) /* a < b   */\
	X( LSH,     LSH,    INF  ) /* a << b  */\
	X( LSHEQ,   LSHEQ,  INF  ) /* a <<= b */\
	X( LTE,     LTE,    INF  ) /* a <= b  */\
	X( ISEQ,    ISEQ,   INF  ) /* a == b  */\
	X( GT,      GT,     INF  ) /* a > b   */\
	X( RSH,     RSH,    INF  ) /* a >> b  */\
	X( RSHEQ,   RSHEQ,  INF  ) /* a >>= b */\
	X( GTE,     GTE,    INF  ) /* a >= b  */

#define X_OPS( X )\
	X_OPS_NOEMIT( X )\
	X_OPS_EMIT( X )

#define X_LEX_1WAY( X )\
	X( '\0', EOS )\
	X( '(',  LP )\
	X( ')',  RP )

#define X_LEX_2WAY( X )\
	X( '!', '=', NOT, NOTEQ )

#define X_LEX_3WAY( X )\
	X( '%', '%', '=', MOD,  ROUND, MODEQ )\
	X( '&', '&', '=', BAND, AND,   BANDEQ )\
	X( '*', '*', '=', MUL,  CEIL,  MULEQ )\
	X( '+', '+', '=', ADD,  INC,   ADDEQ )\
	X( '-', '-', '=', SUB,  DEC,   SUBEQ )\
	X( '/', '/', '=', DIV,  FLOOR, DIVEQ )

#define X_LEX_TYPES( X )\
	X( 0 ... 127,   EOS )\
	X( '0' ... '9', NUM )
	// X( 'A' ... 'Z', ID  )\
	// X( 'a' ... 'z', ID  )\
	// X( '_',         ID )

#define X_LEX_1CASES( CH, TK )\
	case CH: return LexEat( app, TK_##TK );

#define X_LEX_2CASES( CH1, CH2, TK1, TK2 )\
	case CH1:\
		LexEat( app, TK_##TK1 );\
		if( *app->src == CH2 ) return LexEat( app, TK_##TK2 );\
		return;

#define X_LEX_3CASES( CH1, CH2, CH3, TK1, TK2, TK3 )\
	case CH1:\
		LexEat( app, TK_##TK1 );\
		if( *app->src == CH2 ) return LexEat( app, TK_##TK2 );\
		if( *app->src == CH3 ) return LexEat( app, TK_##TK3 );\
		return;

#define X_LEX_TYPE_RANGES( RANGE, TK ) [ RANGE ] = TK_##TK,
#define X_TK_ENUMS( TK, PREC, ASSOC, PREFIX, INFIX, POSTFIX ) TK_##TK,
#define X_TK_PRECS( TK, PREC, ASSOC, PREFIX, INFIX, POSTFIX ) PREC_##PREC,
#define X_TK_ASSOCS( TK, PREC, ASSOC, PREFIX, INFIX, POSTFIX ) ASSOC_##ASSOC,
#define X_PREC_ENUMS( NAME ) PREC_##NAME,
#define X_DENO_ENUMS( E ) DENO_##E,
#define X_DENO_PRES( TK, PREC, ASSOC, PREFIX, INFIX, POSTFIX )  [ POS_PRE ][ TK_##TK ]  = DENO_##PREFIX,
#define X_DENO_INFS( TK, PREC, ASSOC, PREFIX, INFIX, POSTFIX )  [ POS_INF ][ TK_##TK ]  = DENO_##INFIX,
#define X_DENO_POSTS( TK, PREC, ASSOC, PREFIX, INFIX, POSTFIX ) [ POS_POST ][ TK_##TK ] = DENO_##POSTFIX,
#define X_OP_ENUMS( OP, TK, POS ) OP_##OP,
#define X_OP_MAP( OP, TK, POS ) [ POS_##POS ][ TK_##TK ] = OP_##OP,

typedef enum TkType { X_TKS( X_TK_ENUMS ) TK_COUNT } TkType;
typedef enum Prec { X_PRECS( X_PREC_ENUMS ) PREC_COUNT } Prec;
typedef enum Assoc { ASSOC_NONE = 0, ASSOC_LEFT = 0, ASSOC_RIGHT, ASSOC_COUNT } Assoc;
typedef enum Deno { X_DENOS( X_DENO_ENUMS ) DENO_COUNT } Deno;
typedef enum Op { X_OPS( X_OP_ENUMS ) OP_COUNT } Op;
typedef enum Pos { POS_PRE, POS_INF, POS_POST, POS_COUNT } Pos;
typedef char x8; typedef short x16; typedef int x32; typedef long long x64; typedef double f64;
typedef uint8_t u8; typedef uint16_t u16; typedef uint32_t u32; typedef uint64_t u64;
typedef uint8_t Reg;

typedef struct Inst {
	u8 op, a, b, c;
	f64 n;
} Inst;

typedef struct App {
	Inst bc[ BC_MAX ];
	u8* src;
	f64 num;
	TkType tk;
	u16 nbc;
	u8 nr;
} App;

static u8 precs[ ] = { X_TKS( X_TK_PRECS ) };
static u8 assocs[ ] = { X_TKS( X_TK_ASSOCS ) };
static u8 denos[ POS_COUNT ][ TK_COUNT ] = { /* Defines Parse Behavior */
	X_TKS( X_DENO_PRES )
	X_TKS( X_DENO_INFS )
	X_TKS( X_DENO_POSTS )
};
static u8 ops[ POS_COUNT ][ TK_COUNT ] = { /* Defines what to emit */
	X_OPS_EMIT( X_OP_MAP )
};

void Throw( char* fmt, ... ){
	va_list args;
	va_start( args, fmt );
	vfprintf( stderr, fmt, args );
	va_end( args );
	exit( EXIT_FAILURE );
}

void LexEat( App* app, TkType type ){
	app->tk = type;
	++app->src;
}

void LexComment( App* app ){
	++app->src; /* $ */
	for( ; *app->src != '\n'; ++app->src ) if( !*app->src ) break;
}

void LexLt( App* app ){ /* < << <<= <= <== */
	LexEat( app, TK_LT );
	if( *app->src == '<' ){
		LexEat( app, TK_LSH );
		if( *app->src == '=' ) return LexEat( app, TK_LSHEQ );
		return;
	}
	if( *app->src == '=' ){
		LexEat( app, TK_LTE );
		if( *app->src == '=' ) return LexEat( app, TK_BREAK );
		return;
	}
}

void LexEq( App* app ){ /* == ==> */
	LexEat( app, TK_EOS );
	if( *app->src != '=' ) Throw( "Use ':' for assignments.\n" );
	LexEat( app, TK_ISEQ );
	if( *app->src == '>' ) return LexEat( app, TK_CONT );
}

void LexGt( App* app ){ /* > >> >= >>= */
	LexEat( app, TK_GT );
	if( *app->src == '>' ){
		LexEat( app, TK_RSH );
		if( *app->src == '=' ) return LexEat( app, TK_RSHEQ );
		return;
	}
	if( *app->src == '=' ){ LexEat( app, TK_GTE ); }
}

void LexNum( App* app, u8* ops ){
	f64 n = 0;
	for( ; ops[ *app->src ] == TK_NUM; ++app->src )
		n = n * 10.0 + *app->src - '0';
	if( *app->src == '.' && ops[ app->src[ 1 ] ] == TK_NUM ){
		++app->src;
		for( f64 f = 0.1; ops[ *app->src ] == TK_NUM; ++app->src, f *= 0.1 )
			n += ( app->src[ 0 ] - '0' ) * f;
	}
	app->num = n;
	app->tk = TK_NUM;
}

void Lex( App* app ){
	static u8 types[ ] = { X_LEX_TYPES( X_LEX_TYPE_RANGES ) };
	lex:switch( *app->src ){
		default: Throw( "Unexpected Char: '%c'\n", *app->src );
		case 1 ... 32: app->src++; goto lex;
		case '$': LexComment( app ); goto lex;
		case '0' ... '9': return LexNum( app, types );
		case '<': return LexLt( app );
		case '=': return LexEq( app );
		case '>': return LexGt( app );
		// case 'A' ... 'Z': case 'a' ... 'z': case '_': return LexId( app, types );
		X_LEX_1WAY( X_LEX_1CASES )
		X_LEX_2WAY( X_LEX_2CASES )
		X_LEX_3WAY( X_LEX_3CASES )
	}
}

void Match( App* app, TkType tk ){
	if( app->tk != tk ) Throw( "Expected: %d, Got: %d\n", tk, app->tk );
	Lex( app );
}

Reg RegPush( App* app ){
	if( app->nr == REG_MAX - 1 ) Throw( "Register Overflow!\n" );
	return app->nr++;
}

void Emit( App* app, Op op, char a, char b, char c, f64 v ){
	if( app->nbc == BC_MAX ) Throw( "Bytecode Overflow!\n" );
	app->bc[ app->nbc++ ] = ( Inst ){ op, a, b, c, v };
}

Reg Expr( App* a, Prec min );

Reg ExprPrefix( App* a ){
	Reg src = REG_MAX, dst = REG_MAX;
	TkType tk = a->tk;
	f64 num = a->num;
	Lex( a );
	switch( ( Deno )denos[ POS_PRE ][ tk ] ){
	default: Throw( "Bad Expr Prefix: %d\n", tk );
	case DENO_NOP: return src; /* do nothing */
	case DENO_NOPPRE: return Expr( a, PREC_UNARY ); /* ignore unary + */
	case DENO_GRP:
		src = Expr( a, PREC_NONE );
		Match( a, TK_RP );
		return src;
	case DENO_PRE:
		src = Expr( a, PREC_UNARY );
		dst = RegPush( a );
		Emit( a, ops[ POS_PRE ][ tk ], dst, src, 0, num );
		return dst;
	case DENO_NUM:
		dst = RegPush( a );
		Emit( a, OP_LOADC, dst, 0, 0, num );
		return dst;
}}

Reg ExprPostfix( App* app, Reg src ){
	for( ;; ){
		TkType tk = app->tk;
		f64 num = app->num;
		switch( denos[ POS_POST ][ tk ] ){
		default: return src;
		case DENO_POST: /* ++ -- */
			Lex( app );
			Reg dst = RegPush( app );
			Emit( app, ops[ POS_POST ][ tk ], dst, src, 0, num );
			return dst;
		}
	}
}

Reg ExprInfix( App* app, Reg lhs, Prec min ){
	Reg rhs = REG_MAX;
	while( denos[ POS_INF ][ app->tk ] == DENO_INF ){
		if( precs[ app->tk ] - assocs[ app->tk ] <= min ) break;
		TkType tk = app->tk;
		Prec prec = precs[ tk ];
		Lex( app );
		rhs = Expr( app, prec );
		Reg dst = RegPush( app );
		Emit( app, ops[ POS_INF ][ tk ], dst, lhs, rhs, 0 );
		lhs = dst;
	}
	return lhs;
}

Reg Expr( App* app, Prec min ){
	Reg reg = ExprPrefix( app );
	reg = ExprPostfix( app, reg );
	reg = ExprInfix( app, reg, min );
	return reg;
}

void Reset( App* app, u8* src ){
	*app = ( App ){ .src = src, .tk = TK_EOS, .num = 0.0 };
}

void Compile( App* app ){
	Lex( app );
	Reg dst = Expr( app, PREC_NONE );
	if( app->tk != TK_EOS ) Throw( "Unexpected Token: %d\n", app->tk );
	Emit( app, OP_HALT, dst, 0, 0, 0 );
}

f64 Run( Inst* ip ){
	static f64 r[ REG_MAX ];
	static Inst* i;
	RUN: switch( ( i = ip++ )->op ){
	case OP_NOP: goto RUN;
	case OP_HALT: break;
	case OP_LOADC:
		r[ i->a ] = i->n;
		goto RUN;
	case OP_NOT:
		r[ i->a ] = !r[ i->b ];
		goto RUN;
	case OP_NOTEQ:
		r[ i->a ] = r[ i->b ] != r[ i->c ];
		goto RUN;
	case OP_MOD:
		r[ i->a ] = ( x64 )r[ i->b ] % ( x64 )r[ i->c ];
		goto RUN;
	case OP_ROUND:{
		f64 n = r[ i->b ], w = ( x64 )n, f = n - w;
		r[ i->a ] = w + ( f >= .5 ) - ( f <= -.5 );
		goto RUN;
	}
	case OP_MODEQ: /* mutates */
		r[ i->a ] = r[ i->b ] = ( x64 )r[ i->b ] % ( x64 )r[ i->c ];
		goto RUN;
	case OP_BAND:
		r[ i->a ] = ( x64 )r[ i->b ] & ( x64 )r[ i->c ];
		goto RUN;
	case OP_AND:
		r[ i->a ] = r[ i->b ] && r[ i->c ];
		goto RUN;
	case OP_BANDEQ: /* mutates */
		r[ i->a ] = r[ i->b ] = ( x64 )r[ i->b ] & ( x64 )r[ i->c ];
		goto RUN;
	case OP_MUL:
		r[ i->a ] = r[ i->b ] * r[ i->c ];
		goto RUN;
	case OP_CEIL:
		r[ i->a ] = ( x64 )r[ i->b ] + 1;
		goto RUN;
	case OP_MULEQ: /* mutates */
		r[ i->a ] = r[ i->b ] = r[ i->b ] * r[ i->c ];		
		goto RUN;				
	case OP_ADD:
		r[ i->a ] = r[ i->b ] + r[ i->c ];
		goto RUN;
	case OP_PREINC: /* mutates */
		r[ i->b ] = r[ i->b ] + 1;
		r[ i->a ] = r[ i->b ];
		goto RUN;
	case OP_POSTINC: /* mutates */
		r[ i->a ] = r[ i->b ];
		r[ i->b ] = r[ i->b ] + 1;
		goto RUN;
	case OP_ADDEQ: /* mutates */
		r[ i->a ] = r[ i->b ] = r[ i->b ] + r[ i->c ];
		goto RUN;
	case OP_SUB:
		r[ i->a ] = r[ i->b ] - r[ i->c ];
		goto RUN;
	case OP_NEG:
		r[ i->a ] = -r[ i->b ];
		goto RUN;
	case OP_PREDEC: /* mutates */
		r[ i->b ] = r[ i->b ] - 1;
		r[ i->a ] = r[ i->b ];
		goto RUN;	
	case OP_POSTDEC: /* mutates */
		r[ i->a ] = r[ i->b ];
		r[ i->b ] = r[ i->b ] - 1;
		goto RUN;
	case OP_SUBEQ: /* mutates */
		r[ i->a ] = r[ i->b ] = r[ i->b ] - r[ i->c ];
		goto RUN;
	case OP_DIV:
		r[ i->a ] = r[ i->b ] / r[ i->c ];
		goto RUN;
	case OP_FLOOR:
		r[ i->a ] = ( x64 )r[ i->b ];
		goto RUN;
	case OP_DIVEQ: /* mutates */
		r[ i->a ] = r[ i->b ] = r[ i->b ] / r[ i->c ];
		goto RUN;
	case OP_LT:
		r[ i->a ] = r[ i->b ] < r[ i->c ];
		goto RUN;
	case OP_LSH:
		r[ i->a ] = ( x64 )r[ i->b ] << ( x64 )r[ i->c ];
		goto RUN;
	case OP_LSHEQ: /* mutates */
		r[ i->a ] = r[ i->b ] = ( x64 )r[ i->b ] << ( x64 )r[ i->c ];
		goto RUN;
	case OP_LTE:
		r[ i->a ] = r[ i->b ] <= r[ i->c ];
		goto RUN;
	case OP_BREAK:
		goto RUN;
	case OP_ISEQ:
		r[ i->a ] = r[ i->b ] == r[ i->c ];
		goto RUN;
	case OP_CONT:
		goto RUN;		
	case OP_GT:
		r[ i->a ] = r[ i->b ] > r[ i->c ];
		goto RUN;
	case OP_RSH:
		r[ i->a ] = ( x64 )r[ i->b ] >> ( x64 )r[ i->c ];
		goto RUN;
	case OP_RSHEQ: /* mutates */
		r[ i->a ] = r[ i->b ] = ( x64 )r[ i->b ] >> ( x64 )r[ i->c ];
		goto RUN;
	case OP_GTE:
		r[ i->a ] = r[ i->b ] >= r[ i->c ];
		goto RUN;
	}
	return r[ i->a ];
}

void Repl( App* app ){
	u8 src[ REPL_MAX ];
	puts( "Welcome to Repl!" );
	for( ;; ){
		printf( "> " );
		if( !fgets( ( x8* )src, REPL_MAX, stdin ) ) return;
		if( src[ 0 ] == '\n' ) continue;
		Reset( app, src );
		Compile( app );
		f64 n = Run( app->bc );
		printf( "%.2f\n", n );
	}
}

int main( int nargs, char** args ){
	App app;
	Repl( &app );
	return 0;
}