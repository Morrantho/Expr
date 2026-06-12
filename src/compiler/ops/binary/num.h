#ifndef COMPILER_BINARY_NUM_H
#define COMPILER_BINARY_NUM_H

#define X_OPS_BIN_NUM_C( X )\
	X( NOTEQ_NUM,  NotEqNum,  NOTEQ,  NUM, NUM, NUM, "not equal" ) /* a != b */\
	X( MOD_NUM,    ModNum,    MOD,    NUM, NUM, NUM, "modulo" ) /* a % b */\
	X( BAND_NUM,   BandNum,   BAND,   NUM, NUM, NUM, "bitwise and" ) /* a & b */\
	X( AND_NUM,    AndNum,    AND,    NUM, NUM, NUM, "logical and" ) /* a && b */\
	X( MUL_NUM,    MulNum,    MUL,    NUM, NUM, NUM, "multiply" ) /* a * b */\
	X( ADD_NUM,    AddNum,    ADD,    NUM, NUM, NUM, "add" ) /* a + b */\
	X( SUB_NUM,    SubNum,    SUB,    NUM, NUM, NUM, "subtract" ) /* a - b */\
	X( DIV_NUM,    DivNum,    DIV,    NUM, NUM, NUM, "divide" ) /* a / b */\
	X( LT_NUM,     LtNum,     LT,     NUM, NUM, NUM, "less than" ) /* a < b */\
	X( LSH_NUM,    LshNum,    LSH,    NUM, NUM, NUM, "left shift" ) /* a << b */\
	X( LTE_NUM,    LteNum,    LTE,    NUM, NUM, NUM, "less than or equal to" ) /* a <= b */\
	X( ISEQ_NUM,   IsEqNum,   ISEQ,   NUM, NUM, NUM, "equal" ) /* a == b */\
	X( GT_NUM,     GtNum,     GT,     NUM, NUM, NUM, "greater than" ) /* a > b */\
	X( RSH_NUM,    RshNum,    RSH,    NUM, NUM, NUM, "right shift" ) /* a >> b */\
	X( GTE_NUM,    GteNum,    GTE,    NUM, NUM, NUM, "greater than or equal to" ) /* a >= b */\
	X( BXOR_NUM,   BxorNum,   BXOR,   NUM, NUM, NUM, "bitwise xor" ) /* a ^ b */\
	X( BOR_NUM,    BorNum,    BOR,    NUM, NUM, NUM, "bitwise or" ) /* a | b */\
	X( OR_NUM,     OrNum,     OR,     NUM, NUM, NUM, "logical or" ) /* a || b */
#define X_OPS_BIN_NUM_MUT_C( X )\
	X( MODEQ_NUM,  ModEqNum,  MODEQ,  NUM, NUM, NUM, "modulo assign" ) /*  a %= b */\
	X( BANDEQ_NUM, BandEqNum, BANDEQ, NUM, NUM, NUM, "bitwise and assign" ) /* a &= b */\
	X( MULEQ_NUM,  MulEqNum,  MULEQ,  NUM, NUM, NUM, "multiply assign" ) /* a *= b */\
	X( ADDEQ_NUM,  AddEqNum,  ADDEQ,  NUM, NUM, NUM, "add assign" ) /* a += b */\
	X( SUBEQ_NUM,  SubEqNum,  SUBEQ,  NUM, NUM, NUM, "subtract assign" ) /* a -= b */\
	X( DIVEQ_NUM,  DivEqNum,  DIVEQ,  NUM, NUM, NUM, "divide assign" ) /* a /= b */\
	X( LSHEQ_NUM,  LshEqNum,  LSHEQ,  NUM, NUM, NUM, "left shift assign" ) /* a <<= b */\
	X( RSHEQ_NUM,  RshEqNum,  RSHEQ,  NUM, NUM, NUM, "right shift assign" ) /* a >>= b */\
	X( BXOREQ_NUM, BxorEqNum, BXOREQ, NUM, NUM, NUM, "bitwise xor assign" ) /* a ^= b */\
	X( BOREQ_NUM,  BorEqNum,  BOREQ,  NUM, NUM, NUM, "bitwise or assign" ) /* a |= b */
#define X_OPS_BIN_NO_C( X )\
	X( POW_NUM,    PowNum,    POW,    NUM, NUM, NUM, "power" ) /* a ^^ b */

#define X_OPS_BIN( X )\
	X_OPS_BIN_NUM_C( X )\
	X_OPS_BIN_NUM_MUT_C( X )\
	X_OPS_BIN_NO_C( X )

#endif