#ifndef DENO_H
#define DENO_H

#include "../lexer/token.h"
#include "parsepos.h"

#define X_DENOS( X )\
	X( ERR )\
	X( NOP )\
	X( NOPPRE )\
	X( GRP )\
	X( PRE )\
	X( POST )\
	X( INF )\
	X( NUM )

#define X_DENO_ENUMS( ENUM ) DENO_##ENUM,
#define X_DENO_NAMES( ENUM ) ( u8* )#ENUM,
typedef enum Deno { X_DENOS( X_DENO_ENUMS ) DENO_COUNT } Deno;

#define X_DENO_PRES( TK, PREC, ASSOC, PREFIX, INFIX, POSTFIX )\
	[ PARSEPOS_PRE ][ TK_##TK ]  = DENO_##PREFIX,
#define X_DENO_INFS( TK, PREC, ASSOC, PREFIX, INFIX, POSTFIX )\
	[ PARSEPOS_INF ][ TK_##TK ]  = DENO_##INFIX,
#define X_DENO_POSTS( TK, PREC, ASSOC, PREFIX, INFIX, POSTFIX )\
	[ PARSEPOS_POST ][ TK_##TK ] = DENO_##POSTFIX,

u8* DenoGetName( Deno deno );
Deno DenoGet( ParsePos pos, TkType type );

#endif