#ifndef TK_H
#define TK_H

#include "../base/diag/log.h"
#include "../base/intern/intern.h"

#define X_TKS( X ) /* prefix, infix, and postfix point to a denotation type enum */\
	/* ENUM     PREC    ASSOC  PREFIX  INFIX  POSTFIX */\
	X( EOS,     NONE,   NONE,  ERR,    ERR,   ERR  ) /* \0  */\
	X( ERR,     NONE,   NONE,  ERR,    ERR,   ERR  ) /* For parser / compiler hints */\
	X( NOT,     UNARY,  RIGHT, PRE,    ERR,   ERR  ) /* !   */\
	X( NOTEQ,   EQUAL,  LEFT,  ERR,    INF,   ERR  ) /* !=  */\
	X( STR,     NONE,   NONE,  STR,    ERR,   ERR  ) /* "" */\
	X( MOD,     FACTOR, LEFT,  ERR,    INF,   ERR  ) /* %   */\
	X( ROUND,   UNARY,  RIGHT, PRE,    ERR,   ERR  ) /* %%  */\
	X( MODEQ,   ASSIGN, RIGHT, ERR,    INF,   ERR  ) /* %=  */\
	X( BAND,    BAND,   LEFT,  ERR,    INF,   ERR  ) /* &   */\
	X( AND,     AND,    LEFT,  ERR,    INF,   ERR  ) /* &&  */\
	X( BANDEQ,  ASSIGN, RIGHT, ERR,    INF,   ERR  ) /* &=  */\
	X( LP,      NONE,   NONE,  GRP,    ERR,   ERR  ) /* (   */\
	X( RP,      NONE,   NONE,  ERR,    ERR,   ERR  ) /* )   */\
	X( FNCLOSE, NONE,   NONE,  ERR,    ERR,   ERR  ) /* )>  */\
	X( MUL,     FACTOR, LEFT,  ERR,    INF,   ERR  ) /* *   */\
	X( CEIL,    UNARY,  RIGHT, PRE,    ERR,   ERR  ) /* **  */\
	X( MULEQ,   ASSIGN, RIGHT, ERR,    INF,   ERR  ) /* *=  */\
	X( ADD,     TERM,   LEFT,  NOPPRE, INF,   ERR  ) /* +   */\
	X( INC,     UNARY,  LEFT,  PRE,    ERR,   POST ) /* ++  */\
	X( ADDEQ,   ASSIGN, RIGHT, ERR,    INF,   ERR  ) /* +=  */\
	X( SUB,     TERM,   LEFT,  PRE,    INF,   ERR  ) /* -   */\
	X( DEC,     UNARY,  LEFT,  PRE,    ERR,   POST ) /* --  */\
	X( SUBEQ,   ASSIGN, RIGHT, ERR,    INF,   ERR  ) /* -=  */\
	X( MEMBER,  NONE,   NONE,  ERR,    ERR,   POST ) /* .   */\
	X( ELSE,    NONE,   NONE,  ERR,    ERR,   ERR  ) /* ..  */\
	X( IF,      NONE,   NONE,  ERR,    ERR,   ERR  ) /* .:  */\
	X( DIV,     FACTOR, LEFT,  ERR,    INF,   ERR  ) /* /   */\
	X( FLOOR,   UNARY,  RIGHT, PRE,    ERR,   ERR  ) /* //  */\
	X( DIVEQ,   ASSIGN, RIGHT, ERR,    INF,   ERR  ) /* /=  */\
	X( NUM,     NONE,   NONE,  NUM,    ERR,   ERR  ) /* 0-9 */\
	X( ASSIGN,  ASSIGN, NONE,  ERR,    ERR,   ERR  ) /* :   */\
	X( END,     NONE,   NONE,  ERR,    ERR,   ERR  ) /* ::  */\
	X( THEN,    NONE,   NONE,  ERR,    ERR,   ERR  ) /* :.  */\
	X( LOOP,    NONE,   NONE,  ERR,    ERR,   ERR  ) /* ;;  */\
	X( LT,      REL,    LEFT,  ERR,    INF,   ERR  ) /* <   */\
	X( LSH,     SHIFT,  LEFT,  ERR,    INF,   ERR  ) /* <<  */\
	X( LTE,     REL,    LEFT,  ERR,    INF,   ERR  ) /* <=  */\
	X( FNOPEN,  NONE,   NONE,  ERR,    ERR,   ERR  ) /* <(  */\
	X( LSHEQ,   ASSIGN, RIGHT, ERR,    INF,   ERR  ) /* <<= */\
	X( BREAK,   NONE,   NONE,  ERR,    ERR,   ERR  ) /* <== */\
	X( ISEQ,    EQUAL,  LEFT,  ERR,    INF,   ERR  ) /* ==  */\
	X( CONT,    NONE,   NONE,  ERR,    ERR,   ERR  ) /* ==> */\
	X( GT,      REL,    LEFT,  ERR,    INF,   ERR  ) /* >   */\
	X( RSH,     SHIFT,  LEFT,  ERR,    INF,   ERR  ) /* >>  */\
	X( GTE,     REL,    LEFT,  ERR,    INF,   ERR  ) /* >=  */\
	X( RSHEQ,   ASSIGN, RIGHT, ERR,    INF,   ERR  ) /* >>= */\
	X( RET,     NONE,   NONE,  PRE,    ERR,   ERR  ) /* @ */\
	X( ID,      NONE,   NONE,  REF,    ERR,   ERR  ) /* id */\
	X( BXOR,    BXOR,   LEFT,  ERR,    INF,   ERR  ) /* ^   */\
	X( POW,     POW,    LEFT,  ERR,    INF,   ERR  ) /* ^^ */\
	X( BXOREQ,  ASSIGN, RIGHT, ERR,    INF,   ERR  ) /* ^= */\
	X( BOR,     BOR,    LEFT,  ERR,    INF,   ERR  ) /* | */\
	X( OR,      OR,     LEFT,  ERR,    INF,   ERR  ) /* || */\
	X( BOREQ,   ASSIGN, RIGHT, ERR,    INF,   ERR  ) /* |= */\
	X( BNOT,    UNARY,  RIGHT, PRE,    ERR,   ERR  ) /* ~ */

#define X_TK_ENUMS( ENUM, PREC, ASSOC, PRE, INF, POST ) TK_##ENUM,
#define X_TK_STRS( ENUM, PREC, ASSOC, PRE, INF, POST ) ( u8*)#ENUM,

typedef enum TkType { X_TKS( X_TK_ENUMS ) TK_COUNT } TkType;

typedef struct Tk {
	TkType type;
	LogPos pos;
	union {
		f64 num;
		InternId intern;
	};
} Tk;

u8* TkGetName( TkType type );

#endif