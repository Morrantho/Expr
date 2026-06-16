#ifndef EXPR_H
#define EXPR_H

#include "../base/typedefs.h"
#include "../base/intern/intern.h"

typedef u32 Reg;

#define X_EXPRS( X )\
	X( ERR,  "error" )\
	X( NULL, "null" )\
	X( NUM,  "number" )\
	X( STR,  "string" )\
	X( ID,   "id" )\
	X( FUNC, "function" )

#define X_EXPR_ENUMS( ENUM, STR ) EXPR_##ENUM,
#define X_EXPR_STRS( ENUM, STR ) ( u8* )STR,

typedef enum ExprType { X_EXPRS( X_EXPR_ENUMS )  EXPR_COUNT } ExprType;

typedef struct Expr { /* 8 bytes max. If we need more metadata, use u8s for these. */
	ExprType type;
	union {
		InternId intern;
		Reg reg;
	};
} Expr;

u8* ExprGetName( ExprType type );
Expr ExprGen( ExprType type, Reg reg );

#endif