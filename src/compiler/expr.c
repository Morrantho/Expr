#include "expr.h"

u8* ExprGetName( ExprType type ){
	static u8* names[ ] = { X_EXPRS( X_EXPR_STRS ) };
	return names[ type ];
}

Expr ExprGen( ExprType type, Reg reg ){
	return ( Expr ){ .type = type, .reg = reg };
}