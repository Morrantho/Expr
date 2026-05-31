#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define REPL_MAX 255
#define REG_MAX 255
#define BC_MAX 1023

#define X_TOKEN_ENUMS( N, V, PRE, INF, PR, AS, UOP, BOP ) TK_##N = V,
#define X_TOKEN_PRES( N, V, PRE, INF, PR, AS, UOP, BOP ) [ TK_##N ] = DN_##PRE,
#define X_TOKEN_INFS( N, V, PRE, INF, PR, AS, UOP, BOP ) [ TK_##N ] = DN_##INF,
#define X_TOKEN_POSTS( N, V, PRE, INF, PR, AS, UOP, BOP ) [ TK_##N ] = DN_##POST,
#define X_TOKEN_PRECS( N, V, PRE, INF, PR, AS, UOP, BOP ) [ TK_##N ] = PR_##PR,
#define X_TOKEN_ASSOCS( N, V, PRE, INF, PR, AS, UOP, BOP ) [ TK_##N ] = AS_##AS,
#define X_TOKEN_UOPS( N, V, PRE, INF, PR, AS, UOP, BOP ) [ TK_##N ] = OP_##UOP,
#define X_TOKEN_BOPS( N, V, PRE, INF, PR, AS, UOP, BOP ) [ TK_##N ] = OP_##BOP,
#define X_TOKENS( X )\
	/* NAME  VAL PRE  INF  PREC  ASSOC UOP   BOP */\
	X( EOS,  0,  ERR, ERR, NONE, NONE, HALT, HALT )\
	X( NUM,  1,  NUM, ERR, NONE, NONE, HALT, HALT )\
	X( MOD, '%', ERR, BIN, FACT, LEFT, HALT, MOD )\
	X( LP,  '(', GRP, ERR, NONE, NONE, HALT, HALT )\
	X( RP,  ')', ERR, ERR, NONE, NONE, HALT, HALT )\
	X( MUL, '*', ERR, BIN, FACT, LEFT, HALT, MUL )\
	X( ADD, '+', UNA, BIN, TERM, LEFT, HALT, ADD )\
	X( SUB, '-', UNA, BIN, TERM, LEFT, NEG , SUB )\
	X( DIV, '/', ERR, BIN, FACT, LEFT, HALT, DIV )\

typedef uint8_t u8, Reg;
typedef long long x64;
typedef enum TkType { X_TOKENS( X_TOKEN_ENUMS ) } TkType;

typedef enum Deno {
	DN_ERR,
	DN_GRP,
	DN_UNA,
	DN_NUM,
	DN_BIN
} Deno;

typedef enum Prec {
	PR_NONE,
	PR_TERM,
	PR_FACT,
	PR_UNARY
} Prec;

typedef enum Assoc {
	AS_NONE = 0,
	AS_LEFT = 0,
	AS_RIGHT
} Assoc;

typedef enum Op {
	OP_HALT,
	OP_LOADC,
	OP_NEG,
	OP_MOD = '%',
	OP_MUL = '*',
	OP_ADD = '+',
	OP_SUB = '-',
	OP_DIV = '/'
} Op;

typedef struct Inst {
	u8 op, a, b, c;
	double v; /* In a real lang, this would be a const or ref id, sharing fields with a, b, or c */
} Inst;

typedef struct App {
	Inst bc[ BC_MAX ];
	u8* s;
	double n;
	TkType t;
	unsigned short nbc;
	u8 nr;
} App;

static u8 pre[ ] = { X_TOKENS( X_TOKEN_PRES ) };
// static u8 post[ ] = { X_TOKENS( X_TOKEN_POSTS ) };
static u8 inf[ ] = { X_TOKENS( X_TOKEN_INFS ) };
static u8 uops[ ] = { X_TOKENS( X_TOKEN_UOPS ) };
static u8 bops[ ] = { X_TOKENS( X_TOKEN_BOPS ) };
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

void Lex( App* a ){
	static u8 ops[ ] = { [ 0 ... 127 ] = TK_EOS, [ '0' ... '9' ] = TK_NUM };
	LEX: switch( *a->s++ ){
	default: Throw( "stdin: Unexpected Symbol Near '%c'\n", a->s[ -1 ] );
	case '\0': a->t = TK_EOS; return;
	case 1 ... 32: goto LEX;
	case '(': a->t = TK_LP; return;
	case ')': a->t = TK_RP; return;
	case '%': a->t = TK_MOD; return;
	case '*': a->t = TK_MUL; return;
	case '+': a->t = TK_ADD; return;
	case '-': a->t = TK_SUB; return;
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
	Reg r = REG_MAX;
	TkType t = a->t;
	double n = a->n;
	Lex( a );
	switch( pre[ t ] ){
	default: Throw( "Bad Expr Prefix: %d\n", t );
	case DN_GRP:
		r = Expr( a, PR_NONE );
		Match( a, TK_RP );
		return r;
	case DN_UNA:
		r = Expr( a, PR_UNARY );
		Emit( a, uops[ t ], r, r, 0, n );
		return r;
	case DN_NUM:
		r = RegPush( a );
		Emit( a, OP_LOADC, r, 0, 0, n );
		return r;
}}

Reg ExprPostfix( App* a, Reg lhs ){
	return lhs;
}

Reg ExprInfix( App* a, Reg lhs, Prec min ){
	Reg rhs = REG_MAX;
	while( inf[ a->t ] == DN_BIN ){
		if( precs[ a->t ] - assocs[ a->t ] <= min ) break;
		TkType t = a->t;
		Prec p = precs[ t ];
		Lex( a );
		rhs = Expr( a, p );
		Emit( a, bops[ t ], lhs, lhs, rhs, 0 );
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
	Reg r = Expr( a, PR_NONE );
	if( a->t != TK_EOS ) Throw( "Unexpected Token: %d\n", a->t );
	Emit( a, OP_HALT, r, 0, 0, 0 );
}

double Run( Inst* ip ){
	static double r[ REG_MAX ];
	static Inst* i;
	RUN: switch( ( i = ip++ )->op ){
	case OP_HALT: break;
	case OP_LOADC:
		r[ i->a ] = i->v; goto RUN;
	case OP_NEG:
		r[ i->a ] = -r[ i->b ]; goto RUN;
	case OP_MOD:
		r[ i->a ] = ( x64 )r[ i->b ] % ( x64 )r[ i->c ]; goto RUN;		
	case OP_MUL:
		r[ i->a ] = r[ i->b ] * r[ i->c ]; goto RUN;
	case OP_ADD:
		r[ i->a ] = r[ i->b ] + r[ i->c ]; goto RUN;
	case OP_SUB:
		r[ i->a ] = r[ i->b ] - r[ i->c ]; goto RUN;
	case OP_DIV:
		r[ i->a ] = r[ i->b ] / r[ i->c ]; goto RUN;
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