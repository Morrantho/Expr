#ifndef OPCODE_H
#define OPCODE_H

#include "expr.h"
#include "../lexer/token.h"

#define X_OPS_CORE( X )\
	X( ERR,   _, _, _, _, "error" )\
	X( NOP,   _, _, _, _, "no operation" )\
	X( HALT,  _, _, _, _, "halt" )\
	X( LOADC, _, _, _, _, "load constant" ) /* General */
/* UNARY */
#define X_OPS_UNA_NUM_C( X )\
	X( NOT_NUM,	 NOT,  NUM, _, NUM, "logical not" ) /* !a */\
	X( NEG_NUM,	 SUB,  NUM, _, NUM, "negate" ) /* -a */\
	X( BNOT_NUM, BNOT, NUM, _, NUM, "bitwise not" ) /* ~a */
#define X_OPS_UNA_NUM_MUT_C( X )\
	X( PREINC_NUM, INC, NUM, _, NUM, "pre-increment" ) /* ++a */\
	X( PREDEC_NUM, DEC, NUM, _, NUM, "pre-decrement" ) /* --a */
#define X_OPS_UNA_NUM_NO_C( X )\
	X( ROUND_NUM, ROUND, NUM, _, NUM, "round" ) /* %%a */\
	X( CEIL_NUM,  CEIL,  NUM, _, NUM, "ceil" ) /* **a */\
	X( FLOOR_NUM, FLOOR, NUM, _, NUM, "floor" ) /* //a */
/* POSTFIX */
#define X_OPS_POST_NUM_MUT_C( X )\
	X( POSTINC_NUM, INC, NUM, _, NUM, "post-increment" ) /* a++ */\
	X( POSTDEC_NUM, DEC, NUM, _, NUM, "post-decrement" ) /* a-- */
/* BINARY */
#define X_OPS_BIN_NUM_C( X )\
	X( NOTEQ_NUM, NOTEQ, NUM, NUM, NUM, "not equal" ) /* a != b */\
	X( MOD_NUM,   MOD,   NUM, NUM, NUM, "modulo" ) /* a % b */\
	X( BAND_NUM,  BAND,  NUM, NUM, NUM, "bitwise and" ) /* a & b */\
	X( AND_NUM,   AND,   NUM, NUM, NUM, "logical and" ) /* a && b */\
	X( MUL_NUM,   MUL,   NUM, NUM, NUM, "multiply" ) /* a * b */\
	X( ADD_NUM,   ADD,   NUM, NUM, NUM, "add" ) /* a + b */\
	X( SUB_NUM,   SUB,   NUM, NUM, NUM, "subtract" ) /* a - b */\
	X( DIV_NUM,   DIV,   NUM, NUM, NUM, "divide" ) /* a / b */\
	X( LT_NUM,    LT,    NUM, NUM, NUM, "less than" ) /* a < b */\
	X( LSH_NUM,   LSH,   NUM, NUM, NUM, "left shift" ) /* a << b */\
	X( LTE_NUM,   LTE,   NUM, NUM, NUM, "less than or equal to" ) /* a <= b */\
	X( ISEQ_NUM,  ISEQ,  NUM, NUM, NUM, "equal" ) /* a == b */\
	X( GT_NUM,    GT,    NUM, NUM, NUM, "greater than" ) /* a > b */\
	X( RSH_NUM,   RSH,   NUM, NUM, NUM, "right shift" ) /* a >> b */\
	X( GTE_NUM,   GTE,   NUM, NUM, NUM, "greater than or equal to" ) /* a >= b */\
	X( BXOR_NUM,  BXOR,  NUM, NUM, NUM, "bitwise xor" ) /* a ^ b */\
	X( BOR_NUM,   BOR,   NUM, NUM, NUM, "bitwise or" ) /* a | b */\
	X( OR_NUM,    OR,    NUM, NUM, NUM, "logical or" ) /* a || b */
#define X_OPS_BIN_NUM_MUT_C( X )\
	X( MODEQ_NUM,  MODEQ,  NUM, NUM, NUM, "modulo assign" ) /*  a %= b */\
	X( BANDEQ_NUM, BANDEQ, NUM, NUM, NUM, "bitwise and assign" ) /* a &= b */\
	X( MULEQ_NUM,  MULEQ,  NUM, NUM, NUM, "multiply assign" ) /* a *= b */\
	X( ADDEQ_NUM,  ADDEQ,  NUM, NUM, NUM, "add assign" ) /* a += b */\
	X( SUBEQ_NUM,  SUBEQ,  NUM, NUM, NUM, "subtract assign" ) /* a -= b */\
	X( DIVEQ_NUM,  DIVEQ,  NUM, NUM, NUM, "divide assign" ) /* a /= b */\
	X( LSHEQ_NUM,  LSHEQ,  NUM, NUM, NUM, "left shift assign" ) /* a <<= b */\
	X( RSHEQ_NUM,  RSHEQ,  NUM, NUM, NUM, "right shift assign" ) /* a >>= b */\
	X( BXOREQ_NUM, BXOREQ, NUM, NUM, NUM, "bitwise xor assign" ) /* a ^= b */\
	X( BOREQ_NUM,  BOREQ,  NUM, NUM, NUM, "bitwise or assign" ) /* a |= b */
#define X_OPS_BIN_NO_C( X )\
	X( POW_NUM, POW, NUM, NUM, NUM, "power" ) /* a ^^ b */

#define X_OPS_UNA( X )\
	X_OPS_UNA_NUM_C( X )\
	X_OPS_UNA_NUM_MUT_C( X )\
	X_OPS_UNA_NUM_NO_C( X )
#define X_OPS_POST( X )\
	X_OPS_POST_NUM_MUT_C( X )
#define X_OPS_BIN( X )\
	X_OPS_BIN_NUM_C( X )\
	X_OPS_BIN_NUM_MUT_C( X )\
	X_OPS_BIN_NO_C( X )
#define X_OPS( X )\
	X_OPS_CORE( X )\
	X_OPS_UNA( X )\
	X_OPS_POST( X )\
	X_OPS_BIN( X )

#define X_OP_ENUMS( OP, TK, LHS_TYPE, RHS_TYPE, OUT_TYPE, NAME ) OP_##OP,
#define X_OP_STRS( OP, TK, LHS_TYPE, RHS_TYPE, OUT_TYPE, NAME ) ( u8* )#OP,
#define X_OP_UNA_INIT( OP, TK, LHS_TYPE, RHS_TYPE, OUT_TYPE, NAME )\
	[ EXPR_##LHS_TYPE ][ TK_##TK ] = { .code = OP_##OP, .type = EXPR_##OUT_TYPE },
#define X_OP_POST_INIT( OP, TK, LHS_TYPE, RHS_TYPE, OUT_TYPE, NAME )\
	[ EXPR_##LHS_TYPE ][ TK_##TK ] = { .code = OP_##OP, .type = EXPR_##OUT_TYPE },
#define X_OP_BIN_INIT( OP, TK, LHS_TYPE, RHS_TYPE, OUT_TYPE, NAME )\
	[ EXPR_##LHS_TYPE ][ EXPR_##RHS_TYPE ][ TK_##TK ] = { .code = OP_##OP, .type = EXPR_##OUT_TYPE },
#define X_OP_NAME_CASE( OP, TK, LHS_TYPE, RHS_TYPE, OUT_TYPE, NAME )\
	case TK_##TK: return ( u8* )NAME;

typedef enum OpCode { X_OPS( X_OP_ENUMS ) OP_COUNT } OpCode;

typedef struct Op { /* keep these u8 for small tables */
	u8 code;
	u8 type;
} Op;

Op* OpGetUnary( ExprType rhs_type, TkType tk_type );
Op* OpGetPost( ExprType lhs_type, TkType tk_type );
Op* OpGetBinary( ExprType lhs_type, ExprType rhs_type, TkType tk_type );
u8* OpGetName( OpCode opcode );
u8* OpGetUnaryName( TkType tk_type );
u8* OpGetPostName( TkType tk_type );
u8* OpGetBinaryName( TkType tk_type );

#endif