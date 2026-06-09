#ifndef LEXER_H
#define LEXER_H
#include "../base/diag/log.h"
#include "../base/intern/intern.h"
#include "ascii.h"
#include "token.h"
/* Only control flow is allowed in dispatch: */
#define JMP continue
#define RET return
/* computed goto version needs methods even for 1 char tokens. Its annoying, */
/* but it's better than changing the api shape just for LexEat(). */
/* for compatibility, we deal with it. */
#define X_LEX_LABELS( X )\
	X( EOS,     Eos,     RET )\
	X( NEXT,    Next,    JMP )\
	X( LINE,    Line,    JMP )\
	X( NOT,     Not,     RET )\
	X( COMMENT, Comment, JMP )\
	X( MOD,     Mod,     RET )\
	X( BAND,    Band,    RET )\
	X( LP,      Lp,      RET )\
	X( RP,      Rp,      RET )\
	X( MUL,     Mul,     RET )\
	X( ADD,     Add,     RET )\
	X( SUB,     Sub,     RET )\
	X( DIV,     Div,     RET )\
	X( NUM,     Num,     RET )\
	X( LT,      Lt,      RET )\
	X( EQ,      Eq,      RET )\
	X( GT,      Gt,      RET )\
	X( BXOR,    Bxor,    RET )\
	X( BOR,     Bor,     RET )\
	X( BNOT,    Bnot,    RET )

#define X_LEX_TYPE_INIT( CHAR, TK, LABEL, FN, ACTION ) TK_##TK,
/* Switch version. Ignores LABEL. */
#define X_LEX_CASE( CHAR, TK, LABEL, FN, ACTION ) case ASCII_##CHAR:{ Lex##FN( lexer ); ACTION; }
/* Goto Version */
#define X_LEX_LABEL_INIT( CHAR, TK, LABEL, FN, ACTION ) [ ASCII_##CHAR ] = &&LABEL,
#define X_LEX_LABEL( LABEL, FN, ACTION ) LABEL:{ Lex##FN( lexer ); ACTION; }

typedef struct Lexer {
	Logs* logs;		/* App owned */
	Interns* interns;	/* App owned */
	u8* text;
	LogPos pos;
	Tk tk;
} Lexer;

void LexInit( Lexer* lexer, Logs* logs, Interns* interns, SrcId src_id, u8* text );
void LexReset( Lexer* lexer, u8* text );
void Lex( Lexer* lexer );

#endif