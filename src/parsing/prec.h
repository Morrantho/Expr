#ifndef PREC_H
#define PREC_H

#include "../lexer/token.h"

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

#define X_PREC_ENUMS( ENUM ) PREC_##ENUM,
typedef enum Prec { X_PRECS( X_PREC_ENUMS ) PREC_COUNT } Prec;

#define X_TK_PRECS( ENUM, PREC, ASSOC, PRE, INF, POST ) PREC_##PREC,
Prec PrecGet( TkType type );

#endif