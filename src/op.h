#ifdef TYPES
/*EXCLUDE*********************************************************************/
#define X_OPS_EXCLUDE( X )\
	X( ERR,  _, _, _, _, _, "error" )\
	X( HALT, _, _, _, _, _, "halt" )
/*CORE************************************************************************/
#define X_OPS_CORE( X )\
	X( LOADC, LoadConst, _, _, _, _, "load constant" )\
	X( JMP,   Jmp,       _, _, _, _, "unconditional jump" )\
	X( JZ,    Jz,        _, _, _, _, "jump if zero" )\
	X( JNZ,   Jnz,       _, _, _, _, "jump if not zero" )
	//X( RET,   Return,    _, _, _, _, "return" )
	//X( CALL,  Call,      _, _, _, _, "call" )
	//X( ARG,   Arg,       _, _, _, _, "arg" )
/*UNARY***********************************************************************/
#define X_OPS_UNARY_NUM_C( X )\
	X( NOT_NUM,	   NotNum,    NOT,   NUM, _, NUM, "logical not" ) /* !a */\
	X( NEG_NUM,	   NegNum,    SUB,   NUM, _, NUM, "negate" ) /* -a */\
	X( BNOT_NUM,   BnotNum,   BNOT,  NUM, _, NUM, "bitwise not" ) /* ~a */
#define X_OPS_UNARY_NUM_MUT_C( X )\
	X( PREINC_NUM, PreIncNum, INC,   NUM, _, NUM, "pre-increment" ) /* ++a */\
	X( PREDEC_NUM, PreDecNum, DEC,   NUM, _, NUM, "pre-decrement" ) /* --a */
#define X_OPS_UNARY_NUM_NO_C( X )\
	X( ROUND_NUM,  RoundNum,  ROUND, NUM, _, NUM, "round" ) /* %%a */\
	X( CEIL_NUM,   CeilNum,   CEIL,  NUM, _, NUM, "ceil" ) /* **a */\
	X( FLOOR_NUM,  FloorNum,  FLOOR, NUM, _, NUM, "floor" ) /* //a */
#define X_OPS_UNARY( X )\
	X_OPS_UNARY_NUM_C( X )\
	X_OPS_UNARY_NUM_MUT_C( X )\
	X_OPS_UNARY_NUM_NO_C( X )
/*POSTFIX*********************************************************************/
#define X_OPS_POST_NUM_MUT_C( X )\
	X( POSTINC_NUM, PostIncNum, INC, NUM, _, NUM, "post-increment" ) /* a++ */\
	X( POSTDEC_NUM, PostDecNum, DEC, NUM, _, NUM, "post-decrement" ) /* a-- */
#define X_OPS_POST( X )\
	X_OPS_POST_NUM_MUT_C( X )
/*BINARY**********************************************************************/
#define X_OPS_BINARY_NUM_C( X )\
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
#define X_OPS_BINARY_NUM_MUT_C( X )\
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
#define X_OPS_BINARY_NO_C( X )\
	X( POW_NUM,    PowNum,    POW,    NUM, NUM, NUM, "power" ) /* a ^^ b */
#define X_OPS_BINARY( X )\
	X_OPS_BINARY_NUM_C( X )\
	X_OPS_BINARY_NUM_MUT_C( X )\
	X_OPS_BINARY_NO_C( X )
/*ALL*************************************************************************/
#define X_OPS( X )\
	X_OPS_EXCLUDE( X )\
	X_OPS_CORE( X )\
	X_OPS_UNARY( X )\
	X_OPS_POST( X )\
	X_OPS_BINARY( X )
/*INITS***********************************************************************/
#define X_OP_ENUMS( OP, FN, TK, LHS_TYPE, RHS_TYPE, OUT_TYPE, NAME ) OP_##OP,
#define X_OP_STRS( OP, FN, TK, LHS_TYPE, RHS_TYPE, OUT_TYPE, NAME ) ( u8* )#OP,
#define X_OP_NAME_CASE( OP, FN, TK, LHS_TYPE, RHS_TYPE, OUT_TYPE, NAME ) case TK_##TK: return ( u8* )NAME;
#define X_OP_UNARY_INIT( OP, FN, TK, LHS_TYPE, RHS_TYPE, OUT_TYPE, NAME )\
	[ EXPR_##LHS_TYPE ][ TK_##TK ] = { .code = OP_##OP, .type = EXPR_##OUT_TYPE },
#define X_OP_POST_INIT( OP, FN, TK, LHS_TYPE, RHS_TYPE, OUT_TYPE, NAME )\
	[ EXPR_##LHS_TYPE ][ TK_##TK ] = { .code = OP_##OP, .type = EXPR_##OUT_TYPE },
#define X_OP_BINARY_INIT( OP, FN, TK, LHS_TYPE, RHS_TYPE, OUT_TYPE, NAME )\
	[ EXPR_##LHS_TYPE ][ EXPR_##RHS_TYPE ][ TK_##TK ] = { .code = OP_##OP, .type = EXPR_##OUT_TYPE },
#define X_OP_VM_CASE( OP, FN, TK, LHS_TYPE, RHS_TYPE, OUT_TYPE, NAME )\
	case OP_##OP:{ Vm##FN( vm, i ); continue; }
typedef enum OpCode { X_OPS( X_OP_ENUMS ) OP_COUNT } OpCode;
typedef struct Op {
	u8 code; /* OpCode */
	u8 type; /* ExprType */
} Op;
#endif

#ifdef IMPL
Op* OpGetUnary( ExprType rhs_type, TkType tk_type ){
	static Op unary_ops[ EXPR_COUNT ][ TK_COUNT ] = {
		X_OPS_UNARY( X_OP_UNARY_INIT )
	};
	return &unary_ops[ rhs_type ][ tk_type ];
}

Op* OpGetPost( ExprType lhs_type, TkType tk_type ){
	static Op post_ops[ EXPR_COUNT ][ TK_COUNT ] = {
		X_OPS_POST( X_OP_POST_INIT )
	};
	return &post_ops[ lhs_type ][ tk_type ];
}

Op* OpGetBinary( ExprType lhs_type, ExprType rhs_type, TkType tk_type ){
	static Op binary_ops[ EXPR_COUNT ][ EXPR_COUNT ][ TK_COUNT ] = {
		X_OPS_BINARY( X_OP_BINARY_INIT )
	};
	return &binary_ops[ lhs_type ][ rhs_type ][ tk_type ];
}

u8* OpGetName( OpCode opcode ){
	static u8* ops[ ] = { X_OPS( X_OP_STRS ) };
	return ops[ opcode ];
}

u8* OpGetUnaryName( TkType tk_type ){
	switch( tk_type ){
		default: return ( u8* )"unknown operator";
		X_OPS_UNARY( X_OP_NAME_CASE )
	}
}

u8* OpGetPostName( TkType tk_type ){
	switch( tk_type ){
		default: return ( u8* )"unknown operator";
		X_OPS_POST( X_OP_NAME_CASE )
	}
}

u8* OpGetBinaryName( TkType tk_type ){
	switch( tk_type ){
		default: return ( u8* )"unknown operator";
		X_OPS_BINARY( X_OP_NAME_CASE )
	}
}
#endif