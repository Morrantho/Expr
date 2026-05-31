#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define REPL_MAX 255
#define REG_MAX 255
#define BC_MAX 1023

#define X_TOKEN_ENUMS( NAME, PREC, ASSOC, PRE, INF, PST, PRE_OP, INF_OP, PST_OP )   TK_##NAME,
#define X_TOKEN_PRES( NAME, PREC, ASSOC, PRE, INF, PST, PRE_OP, INF_OP, PST_OP )    [ TK_##NAME ] = DENO_##PRE,
#define X_TOKEN_PSTS( NAME, PREC, ASSOC, PRE, INF, PST, PRE_OP, INF_OP, PST_OP )    [ TK_##NAME ] = DENO_##PST,
#define X_TOKEN_INFS( NAME, PREC, ASSOC, PRE, INF, PST, PRE_OP, INF_OP, PST_OP )    [ TK_##NAME ] = DENO_##INF,
#define X_TOKEN_PRECS( NAME, PREC, ASSOC, PRE, INF, PST, PRE_OP, INF_OP, PST_OP )   [ TK_##NAME ] = PREC_##PREC,
#define X_TOKEN_ASSOCS( NAME, PREC, ASSOC, PRE, INF, PST, PRE_OP, INF_OP, PST_OP )  [ TK_##NAME ] = ASSOC_##ASSOC,
#define X_TOKEN_PRE_OPS( NAME, PREC, ASSOC, PRE, INF, PST, PRE_OP, INF_OP, PST_OP ) [ TK_##NAME ] = OP_##PRE_OP,
#define X_TOKEN_PST_OPS( NAME, PREC, ASSOC, PRE, INF, PST, PRE_OP, INF_OP, PST_OP ) [ TK_##NAME ] = OP_##PST_OP,
#define X_TOKEN_INF_OPS( NAME, PREC, ASSOC, PRE, INF, PST, PRE_OP, INF_OP, PST_OP ) [ TK_##NAME ] = OP_##INF_OP,
#define X_TOKENS( X )\
/*     NAME     PREC      ASSOC PRE     INF  PST  PRE_OP INF_OP PST_OP */\
	X( EOS,     NONE,     NONE, ERR,    ERR, ERR, NOP,    NOP,   NOP )\
	X( MOD,     FACT,     LEFT, ERR,    INF, ERR, NOP,    MOD,   NOP )\
	X( LP,      NONE,     NONE, GRP,    ERR, ERR, NOP,    NOP,   NOP )\
	X( RP,      NONE,     NONE, ERR,    ERR, ERR, NOP,    NOP,   NOP )\
	X( MUL,     FACT,     LEFT, ERR,    INF, ERR, NOP,    MUL,   NOP )\
	X( ADD,     TERM,     LEFT, NOP,    INF, ERR, NOP,    ADD,   NOP )\
	X( INC,     PRETERM,  LEFT, PRE,    ERR, PST, PREINC, NOP,   POSTINC )\
	X( SUB,     TERM,     LEFT, PRE,    INF, ERR, NEG,    SUB,   NOP )\
	X( DEC,     PRETERM,  LEFT, PRE,    ERR, PST, PREDEC, NOP,   POSTDEC )\
	X( DIV,     FACT,     LEFT, ERR,    INF, ERR, NOP,    DIV,   NOP )\
	X( NUM,     NONE,     NONE, NUM,    ERR, ERR, NOP,    NOP,   NOP )

typedef uint8_t u8, Reg;
typedef long long x64;
typedef enum TkType { X_TOKENS( X_TOKEN_ENUMS ) } TkType;

typedef enum Deno { /* Denotation */
	DENO_NOP,
	DENO_ERR,
	DENO_GRP,       /* ( */
	DENO_PRE,       /* Unary */
	DENO_PST,       /* Postfix Unary */
	DENO_INF,       /* Binary */
	DENO_NUM,
} Deno;

typedef enum Prec { /* Low To High */
	PREC_NONE,      /* ( */
	PREC_TERM,      /* + - */
	PREC_FACT,      /* * / % */
	PREC_PRE,       /* - ! ~ */
	PREC_PRETERM,   /* ++ -- We dont need a POSTTERM */
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
	OP_ADD,     /* a + b */
	OP_PREINC,  /* ++n */
	OP_POSTINC, /* n++ */
	OP_SUB,     /* a - b */
	OP_NEG,     /* -n */
	OP_PREDEC,  /* --n */
	OP_POSTDEC, /* n-- */
	OP_MUL,     /* a * b */
	OP_DIV,     /* a / b */
	OP_MOD,     /* a % b */
} Op;

typedef struct Inst {
	u8 op, a, b, c;
	double v; /* const / ref id will make it go away soon. */
} Inst;

typedef struct App {
	Inst bc[ BC_MAX ];
	u8* s;
	double n;
	TkType t;
	unsigned short nbc;
	u8 nr;
} App;

static u8 prefixes[ ] = { X_TOKENS( X_TOKEN_PRES ) };
static u8 infixes[ ] = { X_TOKENS( X_TOKEN_INFS ) };
static u8 postfixes[ ] = { X_TOKENS( X_TOKEN_PSTS ) };
static u8 prefix_ops[ ] = { X_TOKENS( X_TOKEN_PRE_OPS ) };
static u8 infix_ops[ ] = { X_TOKENS( X_TOKEN_INF_OPS ) };
static u8 postfix_ops[ ] = { X_TOKENS( X_TOKEN_PST_OPS ) };
static u8 precs[ ] = { X_TOKENS( X_TOKEN_PRECS ) };
static u8 assocs[ ] = { X_TOKENS( X_TOKEN_ASSOCS ) };

void Throw( char* fmt, ... ){
	va_list args;
	va_start( args, fmt );
	vfprintf( stderr, fmt, args );
	va_end( args );
	exit( EXIT_FAILURE );
}

TkType LexNum( App* a, u8* ops ){
	double n = a->s[ -1 ] - '0';
	while( ops[ *a->s ] == TK_NUM ){
		n = n * 10.0 + a->s[ 0 ] - '0';
		++a->s;
	}
	if( *a->s == '.' && ops[ a->s[ 1 ] ] == TK_NUM ){
		++a->s;
		double f = 0.1; /* fraction part */
		while( ops[ *a->s ] == TK_NUM ){
			n += ( a->s[ 0 ] - '0' ) * f;
			f *= 0.1;
			++a->s;
		}
	}
	a->n = n;
	return a->t = TK_NUM;
}

void LexEat( App* a, TkType t ){
	++a->s;
	a->t = t;
}

void LexPlus( App* a ){
	a->t = TK_ADD;
	if( *a->s == '+' ) return LexEat( a, TK_INC );
}

void LexMinus( App* a ){
	a->t = TK_SUB;
	if( *a->s == '-' ) return LexEat( a, TK_DEC );
}

void Lex( App* a ){
	static u8 ops[ ] = { [ 0 ... 127 ] = TK_EOS, [ '0' ... '9' ] = TK_NUM };
	LEX: switch( *a->s++ ){
	default: Throw( "Unexpected Char: '%c'\n", a->s[ -1 ] );
	case '\0': a->t = TK_EOS; return;
	case 1 ... 32: goto LEX; /* Whitespace */
	case '(': a->t = TK_LP; return;
	case ')': a->t = TK_RP; return;
	case '%': a->t = TK_MOD; return;
	case '*': a->t = TK_MUL; return;
	case '+': return LexPlus( a );
	case '-': return LexMinus( a );
	case '/': a->t = TK_DIV; return;
	case '0' ... '9': LexNum( a, ops ); return;
}}

void Match( App* a, TkType t ){
	if( a->t != t ) Throw( "Expected: %d, Got: %d\n", t, a->t );
	Lex( a );
}

Reg RegPush( App* a ){
	if( a->nr == REG_MAX ) Throw( "Register Overflow!\n" );
	return a->nr++;
}

void Emit( App* app, Op op, char a, char b, char c, double v ){
	if( app->nbc == BC_MAX ) Throw( "Bytecode Overflow!\n" );
	app->bc[ app->nbc++ ] = ( Inst ){ op, a, b, c, v };
}

Reg Expr( App* a, Prec min );

Reg ExprPrefix( App* a ){
	Reg src = REG_MAX, dst = REG_MAX;
	TkType t = a->t;
	double n = a->n;
	Lex( a );
	switch( prefixes[ t ] ){
	default: Throw( "Bad Expr Prefix: %d\n", t );
	case DENO_GRP:
		src = Expr( a, PREC_NONE );
		Match( a, TK_RP );
		return src;
	case DENO_PRE: /* Unarys */
		src = Expr( a, PREC_PRE );
		dst = RegPush( a );
		Emit( a, prefix_ops[ t ], dst, src, 0, n );
		return dst;
	case DENO_NOP: /* NOP Prefix Unarys */
		return Expr( a, PREC_PRE );
	case DENO_NUM:
		dst = RegPush( a );
		Emit( a, OP_LOADC, dst, 0, 0, n );
		return dst;
}}

Reg ExprPostfix( App* a, Reg src ){
	for( ;; ){
		TkType t = a->t;
		double n = a->n;
		switch( postfixes[ t ] ){
		default: return src;
		case DENO_PST: /* ++ -- */
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

double Run( Inst* ip ){
	static double r[ REG_MAX ];
	static Inst* i;
	RUN: switch( ( Op )( i = ip++ )->op ){
	case OP_NOP: goto RUN;
	case OP_HALT: break;
	case OP_LOADC: r[ i->a ] = i->v; goto RUN;
	case OP_ADD: r[ i->a ] = r[ i->b ] + r[ i->c ]; goto RUN;
	case OP_PREINC:
		r[ i->b ] = r[ i->b ] + 1;
		r[ i->a ] = r[ i->b ];
		goto RUN;
	case OP_POSTINC:
		r[ i->a ] = r[ i->b ];
		r[ i->b ] = r[ i->b ] + 1;
		goto RUN;
	case OP_SUB: r[ i->a ] = r[ i->b ] - r[ i->c ]; goto RUN;
	case OP_NEG: r[ i->a ] = -r[ i->b ]; goto RUN;
	case OP_PREDEC:
		r[ i->b ] = r[ i->b ] - 1;
		r[ i->a ] = r[ i->b ];
		goto RUN;	
	case OP_POSTDEC:
		r[ i->a ] = r[ i->b ];
		r[ i->b ] = r[ i->b ] - 1;
		goto RUN;
	case OP_MUL: r[ i->a ] = r[ i->b ] * r[ i->c ]; goto RUN;
	case OP_DIV: r[ i->a ] = r[ i->b ] / r[ i->c ]; goto RUN;
	case OP_MOD: r[ i->a ] = ( x64 )r[ i->b ] % ( x64 )r[ i->c ]; goto RUN;		
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
		Reset( app, src );
		Compile( app );
		double v = Run( app->bc );
		printf( "%.2f\n", v );
	}
}

int main( int nargs, char** args ){
	App app;
	Repl( &app );
	return 0;
}