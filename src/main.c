#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "ascii.h"

#define REPL_MAX 255
#define REG_MAX 255
#define BC_MAX 1023

#define X_TOKEN_ENUMS( NAME, PREC, ASSOC, PRE, INF, POST, PRE_OP, INF_OP, POST_OP )   TK_##NAME,
#define X_TOKEN_PRES( NAME, PREC, ASSOC, PRE, INF, POST, PRE_OP, INF_OP, POST_OP )    [ TK_##NAME ] = DENO_##PRE,
#define X_TOKEN_POSTS( NAME, PREC, ASSOC, PRE, INF, POST, PRE_OP, INF_OP, POST_OP )    [ TK_##NAME ] = DENO_##POST,
#define X_TOKEN_INFS( NAME, PREC, ASSOC, PRE, INF, POST, PRE_OP, INF_OP, POST_OP )    [ TK_##NAME ] = DENO_##INF,
#define X_TOKEN_PRECS( NAME, PREC, ASSOC, PRE, INF, POST, PRE_OP, INF_OP, POST_OP )   [ TK_##NAME ] = PREC_##PREC,
#define X_TOKEN_ASSOCS( NAME, PREC, ASSOC, PRE, INF, POST, PRE_OP, INF_OP, POST_OP )  [ TK_##NAME ] = ASSOC_##ASSOC,
#define X_TOKEN_PRE_OPS( NAME, PREC, ASSOC, PRE, INF, POST, PRE_OP, INF_OP, POST_OP ) [ TK_##NAME ] = OP_##PRE_OP,
#define X_TOKEN_POST_OPS( NAME, PREC, ASSOC, PRE, INF, POST, PRE_OP, INF_OP, POST_OP ) [ TK_##NAME ] = OP_##POST_OP,
#define X_TOKEN_INF_OPS( NAME, PREC, ASSOC, PRE, INF, POST, PRE_OP, INF_OP, POST_OP ) [ TK_##NAME ] = OP_##INF_OP,
#define X_TOKENS( X )\
/*     NAME   PREC    ASSOC  PRE     INF  POST  PRE     INF  POST */\
	X( EOS,    NONE,   NONE,  NOP,    ERR, ERR,  NOP,    NOP,   NOP )\
	X( NOT,    UNARY,  LEFT,  PRE,    ERR, ERR,  NOT,    NOP,   NOP )     /* ! */\
	X( NOTEQ,  CMP,    LEFT,  ERR,    INF, ERR,  NOP,    NOTEQ, NOP )     /* != */\
	X( MOD,    FACTOR, NONE,  ERR,    INF, ERR,  NOP,    MOD,   NOP )     /* % */\
	X( ROUND,  UNARY,  LEFT,  PRE,    ERR, ERR,  ROUND,  NOP,   NOP )     /* %% */\
	X( MODEQ,  ASSIGN, RIGHT, ERR,    INF, ERR,  MODEQ,  NOP,   NOP )     /* %= */\
	X( BAND,   BAND,   LEFT,  ERR,    INF, ERR,  NOP,    BAND,  NOP )     /* & */\
	X( AND,    AND,    LEFT,  ERR,    INF, ERR,  NOP,    AND,   NOP )     /* && */\
	X( BANDEQ, AND,    LEFT,  ERR,    INF, ERR,  NOP,    AND,   NOP )     /* &= */\
	X( LP,     NONE,   NONE,  GRP,    ERR, ERR,  NOP,    NOP,   NOP )     /* ( */\
	X( RP,     NONE,   NONE,  ERR,    ERR, ERR,  NOP,    NOP,   NOP )     /* ) */\
	X( MUL,    FACTOR, LEFT,  ERR,    INF, ERR,  NOP,    MUL,   NOP )     /* * */\
	X( CEIL,   UNARY,  NONE,  PRE,    ERR, ERR,  CEIL,   MUL,   NOP )     /* ** */\
	X( MULEQ,  ASSIGN, RIGHT, ERR,    INF, ERR,  NOP,    MULEQ, NOP )     /* *= */\
	X( ADD,    TERM,   LEFT,  NOPPRE, INF, ERR,  NOP,    ADD,   NOP )     /* + */\
	X( INC,    UNARY,  LEFT,  PRE,    ERR, POST, PREINC, NOP,   POSTINC ) /* ++ */\
	X( ADDEQ,  ASSIGN, RIGHT, ERR,    INF, ERR,  NOP,    ADDEQ, NOP )     /* += */\
	X( SUB,    TERM,   LEFT,  PRE,    INF, ERR,  NEG,    SUB,   NOP )     /* - */\
	X( DEC,    UNARY,  LEFT,  PRE,    ERR, POST, PREDEC, NOP,   POSTDEC ) /* -- */\
	X( SUBEQ,  ASSIGN, RIGHT, ERR,    INF, ERR,  NOP,    SUBEQ, NOP )     /* -= */\
	X( DIV,    FACTOR, LEFT,  ERR,    INF, ERR,  NOP,    DIV,   NOP )     /* / */\
	X( FLOOR,  UNARY,  NONE,  PRE,    ERR, ERR,  FLOOR,  NOP,   NOP )     /* // */\
	X( DIVEQ,  ASSIGN, RIGHT, ERR,    INF, ERR,  NOP,    DIVEQ, NOP )     /* /= */\
	X( NUM,    NONE,   NONE,  NUM,    ERR, ERR,  NOP,    NOP,   NOP )     /* 0-9.0-9 */

#define X_LEX_TYPE_RANGES( RANGE, TK ) [ RANGE ] = TK_##TK,
#define X_LEX_TYPES( X )\
	X( 0 ... 127, EOS )\
	X( '0' ... '9', NUM )

typedef char x8;
typedef long long x64;
typedef double f64;
typedef uint8_t u8, Reg;
typedef enum TkType { X_TOKENS( X_TOKEN_ENUMS ) } TkType;

typedef enum Deno { /* Denotation */
	DENO_NOP,
	DENO_NOPPRE,
	DENO_ERR,
	DENO_GRP,       /* ( */
	DENO_PRE,       /* Unary */
	DENO_POST,       /* Postfix Unary */
	DENO_INF,       /* Binary */
	DENO_NUM,
} Deno;

typedef enum Prec {
	PREC_NONE,
	PREC_ERR,		/* Needs to exist since we sub 1 for right assocs */
	PREC_ASSIGN,	/* = += -= *= /= %= &= |= ^= <<= >>= */
	PREC_OR,		/* || */
	PREC_AND,		/* && */
	PREC_CMP,		/* == != < <= > >= */
	PREC_BOR,		/* | */
	PREC_BXOR,		/* ^ */
	PREC_BAND,		/* & */
	PREC_SHIFT,		/* << >> */
	PREC_TERM,		/* + - */
	PREC_FACTOR,	/* * / % */
	PREC_UNARY,		/* ! # ~ ++ -- // ** %% - */
	PREC_POW		/* ^^ */
	/* a.b arr[ ] fn( ) ++ -- */
} Prec;

typedef enum Assoc {
	ASSOC_NONE = 0,
	ASSOC_LEFT = 0,
	ASSOC_RIGHT
} Assoc;

typedef enum Op {
	OP_NOP,
	OP_HALT,
	OP_LOADC,
	OP_NOT,		/* !a */
	OP_NOTEQ,	/* a != b */
	OP_ADD,     /* a + b */
	OP_PREINC,  /* ++n */
	OP_POSTINC, /* n++ */
	OP_ADDEQ,   /* a += b */
	OP_SUB,     /* a - b */
	OP_NEG,     /* -n */
	OP_PREDEC,  /* --n */
	OP_POSTDEC, /* n-- */
	OP_SUBEQ,	/* a -= b */
	OP_MUL,     /* a * b */
	OP_CEIL,	/* a ** b */
	OP_MULEQ,	/* a *= b */
	OP_DIV,     /* a / b */
	OP_FLOOR,	/* //a */
	OP_DIVEQ,	/* a /= b */
	OP_MOD,     /* a % b */
	OP_ROUND,	/* %%a */
	OP_MODEQ,	/* a %= b */
	OP_BAND,	/* a & b */
	OP_AND,		/* a && b */
	OP_BANDEQ	/* a &= b */
} Op;

typedef struct Inst {
	u8 op, a, b, c;
	f64 v; /* const / ref id will make it go away soon. */
} Inst;

typedef struct App {
	Inst bc[ BC_MAX ];
	u8* s;
	f64 n;
	TkType t;
	unsigned short nbc;
	u8 nr;
} App;

static u8 prefixes[ ] = { X_TOKENS( X_TOKEN_PRES ) };
static u8 infixes[ ] = { X_TOKENS( X_TOKEN_INFS ) };
static u8 postfixes[ ] = { X_TOKENS( X_TOKEN_POSTS ) };
static u8 precs[ ] = { X_TOKENS( X_TOKEN_PRECS ) };
static u8 assocs[ ] = { X_TOKENS( X_TOKEN_ASSOCS ) };
static u8 prefix_ops[ ] = { X_TOKENS( X_TOKEN_PRE_OPS ) };
static u8 infix_ops[ ] = { X_TOKENS( X_TOKEN_INF_OPS ) };
static u8 postfix_ops[ ] = { X_TOKENS( X_TOKEN_POST_OPS ) };

void Throw( char* fmt, ... ){
	va_list args;
	va_start( args, fmt );
	vfprintf( stderr, fmt, args );
	va_end( args );
	exit( EXIT_FAILURE );
}

void LexEat( App* a, TkType t ){
	a->t = t;
	++a->s;
}

void LexNot( App* a ){
	LexEat( a, TK_NOT );
	if( *a->s == '=' ) return LexEat( a, TK_NOTEQ );
}

void LexComment( App* a ){
	while( *a->s == '$' ) a->s++;
}

void LexMod( App* a ){
	LexEat( a, TK_MOD );
	if( *a->s == '%' ) return LexEat( a, TK_ROUND );
	if( *a->s == '=' ) return LexEat( a, TK_MODEQ );
}

void LexBand( App* a ){
	LexEat( a, TK_BAND );
	if( *a->s == '&' ) return LexEat( a, TK_AND );
	if( *a->s == '=' ) return LexEat( a, TK_BANDEQ );
}

void LexMul( App* a ){
	LexEat( a, TK_MUL );
	if( *a->s == '*' ) return LexEat( a, TK_CEIL );
	if( *a->s == '=' ) return LexEat( a, TK_MULEQ );
}

void LexPlus( App* a ){
	LexEat( a, TK_ADD );
	if( *a->s == '+' ) return LexEat( a, TK_INC );
	if( *a->s == '=' ) return LexEat( a, TK_ADDEQ );
}

void LexMinus( App* a ){
	LexEat( a, TK_SUB );
	if( *a->s == '-' ) return LexEat( a, TK_DEC );
	if( *a->s == '=' ) return LexEat( a, TK_SUBEQ );
}

void LexDiv( App* a ){
	LexEat( a, TK_DIV );
	if( *a->s == '/' ) return LexEat( a, TK_FLOOR );
	if( *a->s == '=' ) return LexEat( a, TK_DIVEQ );
}

void LexNum( App* a, u8* ops ){
	f64 n = 0;
	for( ; ops[ *a->s ] == TK_NUM; ++a->s )
		n = n * 10.0 + *a->s - '0';
	if( *a->s == '.' && ops[ a->s[ 1 ] ] == TK_NUM ){
		++a->s;
		for( f64 f = 0.1; ops[ *a->s ] == TK_NUM; ++a->s, f *= 0.1 )
			n += ( a->s[ 0 ] - '0' ) * f;
	}
	a->n = n;
	a->t = TK_NUM;
}

void Lex( App* a ){
	static u8 ops[ ] = { X_LEX_TYPES( X_LEX_TYPE_RANGES ) };
	LEX: switch( *a->s ){
	default: Throw( "Unexpected Char: '%c'\n", *a->s );
	case '\0': return LexEat( a, TK_EOS );
	case 1 ... 32: a->s++; goto LEX;
	case '!': return LexNot( a );
	// case '\"': return LexStr( a, ops );
	case '$': LexComment( a ); goto LEX;
	case '(': return LexEat( a, TK_LP );
	case ')': return LexEat( a, TK_RP );
	case '%': return LexMod( a );
	case '&': return LexBand( a );
	case '*': return LexMul( a );
	case '+': return LexPlus( a );
	case '-': return LexMinus( a );
	case '/': return LexDiv( a );
	case '0' ... '9': return LexNum( a, ops );
}}

void Match( App* a, TkType t ){
	if( a->t != t ) Throw( "Expected: %d, Got: %d\n", t, a->t );
	Lex( a );
}

Reg RegPush( App* a ){
	if( a->nr == REG_MAX ) Throw( "Register Overflow!\n" );
	return a->nr++;
}

void Emit( App* app, Op op, char a, char b, char c, f64 v ){
	if( app->nbc == BC_MAX ) Throw( "Bytecode Overflow!\n" );
	app->bc[ app->nbc++ ] = ( Inst ){ op, a, b, c, v };
}

Reg Expr( App* a, Prec min );

Reg ExprPrefix( App* a ){
	Reg src = REG_MAX, dst = REG_MAX;
	TkType t = a->t;
	f64 n = a->n;
	Lex( a );
	switch( ( Deno )prefixes[ t ] ){
	default: Throw( "Bad Expr Prefix: %d\n", t );
	case DENO_NOP: return src; /* True NOP */
	case DENO_NOPPRE: return Expr( a, PREC_UNARY ); /* NOP + */
	case DENO_GRP:
		src = Expr( a, PREC_NONE );
		Match( a, TK_RP );
		return src;
	case DENO_PRE:
		src = Expr( a, PREC_UNARY );
		dst = RegPush( a );
		Emit( a, prefix_ops[ t ], dst, src, 0, n );
		return dst;
	case DENO_NUM:
		dst = RegPush( a );
		Emit( a, OP_LOADC, dst, 0, 0, n );
		return dst;
}}

Reg ExprPostfix( App* a, Reg src ){
	for( ;; ){
		TkType t = a->t;
		f64 n = a->n;
		switch( postfixes[ t ] ){
		default: return src;
		case DENO_POST: /* ++ -- */
			Lex( a );
			Reg dst = RegPush( a );
			Emit( a, postfix_ops[ t ], dst, src, 0, n );
			return dst;
		}
	}
}

Reg ExprInfix( App* a, Reg lhs, Prec min ){
	Reg rhs = REG_MAX;
	while( infixes[ a->t ] == DENO_INF ){
		if( precs[ a->t ] - assocs[ a->t ] <= min ) break;
		TkType t = a->t;
		Prec p = precs[ t ];
		Lex( a );
		rhs = Expr( a, p );
		Reg dst = RegPush( a );
		Emit( a, infix_ops[ t ], dst, lhs, rhs, 0 );
		lhs = dst;
	}
	return lhs;
}

Reg Expr( App* a, Prec min ){
	Reg r = ExprPrefix( a );
	r = ExprPostfix( a, r );
	r = ExprInfix( a, r, min );
	return r;
}

void Compile( App* a ){
	Lex( a );
	Reg dst = Expr( a, PREC_NONE );
	if( a->t != TK_EOS ) Throw( "Unexpected Token: %d\n", a->t );
	Emit( a, OP_HALT, dst, 0, 0, 0 );
}

f64 Run( Inst* ip ){
	static f64 r[ REG_MAX ];
	static Inst* i;
	RUN: switch( ( Op )( i = ip++ )->op ){
	case OP_NOP: goto RUN;
	case OP_HALT: break;
	case OP_LOADC:
		r[ i->a ] = i->v;
		goto RUN;
	case OP_NOT:
		r[ i->a ] = !r[ i->b ];
		goto RUN;
	case OP_NOTEQ:
		r[ i->a ] = r[ i->b ] != r[ i->c ];
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
	case OP_MUL:
		r[ i->a ] = r[ i->b ] * r[ i->c ];
		goto RUN;
	case OP_CEIL:
		r[ i->a ] = ( x64 )r[ i->b ] + 1;
		goto RUN;
	case OP_MULEQ: /* mutates */
		r[ i->a ] = r[ i->b ] = r[ i->b ] * r[ i->c ];		
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
	}
	return r[ i->a ];
}

void Reset( App* app, u8* src ){
	app->s = src;
	app->n = 0;
	app->t = TK_EOS;
	app->nr = 0;
	app->nbc = 0;
}

void Repl( App* app ){
	u8 src[ REPL_MAX ];
	printf( "Welcome to Repl!\n" );
	for( ;; ){
		printf( "> " );
		if( !fgets( ( char* )src, REPL_MAX, stdin ) ) return;
		if( src[ 0 ] == '\n' ) continue;
		Reset( app, src );
		Compile( app );
		f64 v = Run( app->bc );
		printf( "%.2f\n", v );
	}
}

int main( int nargs, char** args ){
	App app;
	Repl( &app );
	return 0;
}