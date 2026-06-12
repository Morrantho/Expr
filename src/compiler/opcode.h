#ifndef OPCODE_H
#define OPCODE_H

#include "expr.h"
#include "../lexer/token.h"
#include "ops/ops.h"

#define X_OP_ENUMS( OP, FN, TK, LHS_TYPE, RHS_TYPE, OUT_TYPE, NAME ) OP_##OP,
#define X_OP_STRS( OP, FN, TK, LHS_TYPE, RHS_TYPE, OUT_TYPE, NAME ) ( u8* )#OP,
#define X_OP_NAME_CASE( OP, FN, TK, LHS_TYPE, RHS_TYPE, OUT_TYPE, NAME )\
	case TK_##TK: return ( u8* )NAME;

typedef enum OpCode { X_OPS( X_OP_ENUMS ) OP_COUNT } OpCode;

typedef struct Op { /* keep these u8 for small tables */
	u8 code;
	u8 type;
} Op;

Op* OpGetUnary( ExprType rhs_type, TkType tk_type );
Op* OpGetPost( ExprType lhs_type, TkType tk_type );
Op* OpGetBinary( ExprType lhs_type, ExprType rhs_type, TkType tk_type );
/* For Logging: */
u8* OpGetName( OpCode opcode );
u8* OpGetUnaryName( TkType tk_type );
u8* OpGetPostName( TkType tk_type );
u8* OpGetBinaryName( TkType tk_type );

#endif