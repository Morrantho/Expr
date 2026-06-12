#ifndef COMPILER_UNARY_H
#define COMPILER_UNARY_H

#include "num.h"

#define X_OP_UNA_INIT( OP, FN, TK, LHS_TYPE, RHS_TYPE, OUT_TYPE, NAME )\
	[ EXPR_##LHS_TYPE ][ TK_##TK ] = { .code = OP_##OP, .type = EXPR_##OUT_TYPE },

#endif