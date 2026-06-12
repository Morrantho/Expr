#ifndef COMPILER_BINARY_H
#define COMPILER_BINARY_H

#include "num.h"

#define X_OP_BIN_INIT( OP, FN, TK, LHS_TYPE, RHS_TYPE, OUT_TYPE, NAME )\
	[ EXPR_##LHS_TYPE ][ EXPR_##RHS_TYPE ][ TK_##TK ] = { .code = OP_##OP, .type = EXPR_##OUT_TYPE },

#endif