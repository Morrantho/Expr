#ifndef ASSOC_H
#define ASSOC_H

#include "../lexer/token.h"

typedef enum Assoc {
	ASSOC_NONE = 0,
	ASSOC_LEFT = 1,
	ASSOC_RIGHT,
	ASSOC_COUNT
} Assoc;

#define X_TK_ASSOCS( ENUM, PREC, ASSOC, PRE, INF, POST ) ASSOC_##ASSOC,
Assoc AssocGet( TkType type );

#endif