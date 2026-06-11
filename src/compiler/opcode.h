#ifndef OPCODE_H
#define OPCODE_H

#define X_OPS_CORE( X )\
	X( NOP,   _, _, _, )\
	X( HALT,  _, _, _, )\
	X( LOADC, _, _, _, ) /* General */

#define X_OPS_UNA_NUM_C( X )\
	X( NOT_NUM,	 NOT,  NUM, _, NUM ) /* !a */\
	X( NEG_NUM,	 NEG,  NUM, _, NUM ) /* -a */\
	X( BNOT_NUM, BNOT, NUM, _, NUM ) /* ~a */
	
#define X_OPS_UNA_NUM_MUT_PRE_C( X )\
	X( PREINC_NUM, INC, NUM, _, NUM ) /* ++a */\
	X( PREDEC_NUM, DEC, NUM, _, NUM ) /* --a */

#define X_OPS_UNA_NUM_MUT_POST_C( X )\
	X( POSTINC_NUM, INC, NUM, _, NUM ) /* a++ */\
	X( POSTDEC_NUM, DEC, NUM, _, NUM ) /* a-- */

#define X_OPS_UNA_NUM_NO_C( X )\
	X( ROUND_NUM, ROUND, NUM, _, NUM ) /* %%a */\
	X( CEIL_NUM,  CEIL,  NUM, _, NUM ) /* **a */\
	X( FLOOR_NUM, FLOOR, NUM, _, NUM ) /* //a */

#define X_OPS_UNA( X )\
	X_OPS_UNA_NUM_C( X )\
	X_OPS_UNA_NUM_MUT_PRE_C( X )\
	X_OPS_UNA_NUM_NO_C( X )
	
#define X_OPS_POST( X )\
	X_OPS_UNA_NUM_MUT_POST_C( X )

#define X_OPS_BIN_NUM_C( X )\
	X( NOTEQ_NUM, NOTEQ, NUM, NUM, NUM ) /* a != b */\
	X( MOD_NUM,   MOD,   NUM, NUM, NUM ) /* a % b */\
	X( BAND_NUM,  BAND,  NUM, NUM, NUM ) /* a & b */\
	X( AND_NUM,   AND,   NUM, NUM, NUM ) /* a && b */\
	X( MUL_NUM,   MUL,   NUM, NUM, NUM ) /* a * b */\
	X( ADD_NUM,   ADD,   NUM, NUM, NUM ) /* a + b */\
	X( SUB_NUM,   SUB,   NUM, NUM, NUM ) /* a - b */\
	X( DIV_NUM,   DIV,   NUM, NUM, NUM ) /* a / b */\
	X( LT_NUM,    LT,    NUM, NUM, NUM ) /* a < b */\
	X( LSH_NUM,   LSH,   NUM, NUM, NUM ) /* a << b */\
	X( LTE_NUM,   LTE,   NUM, NUM, NUM ) /* a <= b */\
	X( ISEQ_NUM,  ISEQ,  NUM, NUM, NUM ) /* a == b */\
	X( GT_NUM,    GT,    NUM, NUM, NUM ) /* a > b */\
	X( RSH_NUM,   RSH,   NUM, NUM, NUM ) /* a >> b */\
	X( GTE_NUM,   GTE,   NUM, NUM, NUM ) /* a >= b */\
	X( BXOR_NUM,  BXOR,  NUM, NUM, NUM ) /* a ^ b */\
	X( BOR_NUM,   BOR,   NUM, NUM, NUM ) /* a | b */\
	X( OR_NUM,    OR,    NUM, NUM, NUM ) /* a || b */

#define X_OPS_BIN_NUM_MUT_C( X )\
	X( MODEQ_NUM,  MODEQ,  NUM, NUM, NUM ) /*  a %= b */\
	X( BANDEQ_NUM, BANDEQ, NUM, NUM, NUM ) /* a &= b */\
	X( MULEQ_NUM,  MULEQ,  NUM, NUM, NUM ) /* a *= b */\
	X( ADDEQ_NUM,  ADDEQ,  NUM, NUM, NUM ) /* a += b */\
	X( DIVEQ_NUM,  DIVEQ,  NUM, NUM, NUM ) /* a /= b */\
	X( LSHEQ_NUM,  LSHEQ,  NUM, NUM, NUM ) /* a <<= b */\
	X( RSHEQ_NUM,  RSHEQ,  NUM, NUM, NUM ) /* a >>= b */\
	X( BXOREQ_NUM, BXOREQ, NUM, NUM, NUM ) /* a ^= b */\
	X( BOREQ_NUM,  BOREQ,  NUM, NUM, NUM ) /* a |= b */

#define X_OPS_BIN_NO_C( X )\
	X( POW_NUM, POW, NUM, NUM, NUM ) /* a ^^ b */

#define X_OPS_BIN( X )\
	X_OPS_BIN_NUM_C( X )\
	X_OPS_BIN_NUM_MUT_C( X )\
	X_OPS_BIN_NO_C( X )

#define X_OPS( X )\
	X_OPS_CORE( X )\
	X_OPS_UNA( X )\
	X_OPS_POST( X )\
	X_OPS_BIN( X )

#define X_OP_ENUMS( OP, TK, LHS_TYPE, RHS_TYPE, OUT_TYPE ) OP_##OP,
typedef enum OpCode { X_OPS( X_OP_ENUMS ) OP_COUNT } OpCode;

#endif