#ifndef COMPILER_UNARY_NUM_H
#define COMPILER_UNARY_NUM_H

#define X_OPS_UNA_NUM_C( X )\
	X( NOT_NUM,	   NotNum,    NOT,   NUM, _, NUM, "logical not" ) /* !a */\
	X( NEG_NUM,	   NegNum,    SUB,   NUM, _, NUM, "negate" ) /* -a */\
	X( BNOT_NUM,   BnotNum,   BNOT,  NUM, _, NUM, "bitwise not" ) /* ~a */
#define X_OPS_UNA_NUM_MUT_C( X )\
	X( PREINC_NUM, PreIncNum, INC,   NUM, _, NUM, "pre-increment" ) /* ++a */\
	X( PREDEC_NUM, PreDecNum, DEC,   NUM, _, NUM, "pre-decrement" ) /* --a */
#define X_OPS_UNA_NUM_NO_C( X )\
	X( ROUND_NUM,  RoundNum,  ROUND, NUM, _, NUM, "round" ) /* %%a */\
	X( CEIL_NUM,   CeilNum,   CEIL,  NUM, _, NUM, "ceil" ) /* **a */\
	X( FLOOR_NUM,  FloorNum,  FLOOR, NUM, _, NUM, "floor" ) /* //a */

#define X_OPS_UNA( X )\
	X_OPS_UNA_NUM_C( X )\
	X_OPS_UNA_NUM_MUT_C( X )\
	X_OPS_UNA_NUM_NO_C( X )

#endif