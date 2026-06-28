#ifdef TYPES
/*EXCLUDE*********************************************************************/
#define X_OPS_EXCLUDE( X )\
	X( ERR,  _, _, _, _, _ )\
	X( HALT, _, _, _, _, _ )
/*CORE************************************************************************/
#define X_OPS_CORE( X )\
	X( LOADC,  LoadConst, _, _, _, _  )\
	X( JMP,    Jmp,       _, _, _, _ )\
	X( JZ,     Jz,        _, _, _, _ )\
	X( JNZ,    Jnz,       _, _, _, _ )\
	X( MOV,    Mov,       _, _, _, _ )\
	X( RET,    Return,    _, _, _, _ )\
	X( CALL,   Call,      _, _, _, _ )
/*UNARY***********************************************************************/
#define X_OPS_UNARY_NUM_C( X )\
	X( NOT_NUM,	   NotNum,    NOT,   NUM, _, NUM ) /* !a */\
	X( NEG_NUM,	   NegNum,    SUB,   NUM, _, NUM ) /* -a */\
	X( BNOT_NUM,   BnotNum,   BNOT,  NUM, _, NUM ) /* ~a */
#define X_OPS_UNARY_NUM_MUT_C( X )\
	X( PREINC_NUM, PreIncNum, INC,   NUM, _, NUM ) /* ++a */\
	X( PREDEC_NUM, PreDecNum, DEC,   NUM, _, NUM ) /* --a */
#define X_OPS_UNARY_NUM_NO_C( X )\
	X( ROUND_NUM,  RoundNum,  ROUND, NUM, _, NUM ) /* %%a */\
	X( CEIL_NUM,   CeilNum,   CEIL,  NUM, _, NUM ) /* **a */\
	X( FLOOR_NUM,  FloorNum,  FLOOR, NUM, _, NUM ) /* //a */
#define X_OPS_UNARY( X )\
	X_OPS_UNARY_NUM_C( X )\
	X_OPS_UNARY_NUM_MUT_C( X )\
	X_OPS_UNARY_NUM_NO_C( X )
/*POSTFIX*********************************************************************/
#define X_OPS_POST_NUM_MUT_C( X )\
	X( POSTINC_NUM, PostIncNum, INC, NUM, _, NUM ) /* a++ */\
	X( POSTDEC_NUM, PostDecNum, DEC, NUM, _, NUM ) /* a-- */
#define X_OPS_POST( X )\
	X_OPS_POST_NUM_MUT_C( X )
/*BINARY NUM******************************************************************/
#define X_OPS_BINARY_NUM_C( X )\
	X( NOTEQ_NUM,  NotEqNum,  NOTEQ,  NUM, NUM, NUM ) /* a != b */\
	X( MOD_NUM,    ModNum,    MOD,    NUM, NUM, NUM ) /* a % b */\
	X( BAND_NUM,   BandNum,   BAND,   NUM, NUM, NUM ) /* a & b */\
	X( AND_NUM,    AndNum,    AND,    NUM, NUM, NUM ) /* a && b */\
	X( MUL_NUM,    MulNum,    MUL,    NUM, NUM, NUM ) /* a * b */\
	X( ADD_NUM,    AddNum,    ADD,    VALUE, VALUE, VALUE ) /* a + b */\
	X( SUB_NUM,    SubNum,    SUB,    NUM, NUM, NUM ) /* a - b */\
	X( DIV_NUM,    DivNum,    DIV,    NUM, NUM, NUM ) /* a / b */\
	X( LT_NUM,     LtNum,     LT,     NUM, NUM, NUM ) /* a < b */\
	X( LSH_NUM,    LshNum,    LSH,    NUM, NUM, NUM ) /* a << b */\
	X( LTE_NUM,    LteNum,    LTE,    NUM, NUM, NUM ) /* a <= b */\
	X( CMP_NUM,    CmpNum,    CMP,    NUM, NUM, NUM ) /* a == b */\
	X( GT_NUM,     GtNum,     GT,     NUM, NUM, NUM ) /* a > b */\
	X( RSH_NUM,    RshNum,    RSH,    NUM, NUM, NUM ) /* a >> b */\
	X( GTE_NUM,    GteNum,    GTE,    NUM, NUM, NUM ) /* a >= b */\
	X( BXOR_NUM,   BxorNum,   BXOR,   NUM, NUM, NUM ) /* a ^ b */\
	X( BOR_NUM,    BorNum,    BOR,    NUM, NUM, NUM ) /* a | b */\
	X( OR_NUM,     OrNum,     OR,     NUM, NUM, NUM ) /* a || b */
#define X_OPS_BINARY_NUM_MUT_C( X )\
	X( MODEQ_NUM,  ModEqNum,  MODEQ,  NUM, NUM, NUM ) /*  a %= b */\
	X( BANDEQ_NUM, BandEqNum, BANDEQ, NUM, NUM, NUM ) /* a &= b */\
	X( MULEQ_NUM,  MulEqNum,  MULEQ,  NUM, NUM, NUM ) /* a *= b */\
	X( ADDEQ_NUM,  AddEqNum,  ADDEQ,  NUM, NUM, NUM ) /* a += b */\
	X( SUBEQ_NUM,  SubEqNum,  SUBEQ,  NUM, NUM, NUM ) /* a -= b */\
	X( DIVEQ_NUM,  DivEqNum,  DIVEQ,  NUM, NUM, NUM ) /* a /= b */\
	X( LSHEQ_NUM,  LshEqNum,  LSHEQ,  NUM, NUM, NUM ) /* a <<= b */\
	X( RSHEQ_NUM,  RshEqNum,  RSHEQ,  NUM, NUM, NUM ) /* a >>= b */\
	X( BXOREQ_NUM, BxorEqNum, BXOREQ, NUM, NUM, NUM ) /* a ^= b */\
	X( BOREQ_NUM,  BorEqNum,  BOREQ,  NUM, NUM, NUM ) /* a |= b */
#define X_OPS_BINARY_NO_C( X )\
	X( POW_NUM,    PowNum,    POW,    NUM, NUM, NUM ) /* a ^^ b */
/*BINARY STR******************************************************************/
#define X_OPS_BINARY_STR_C( X )\
	X( CMP_STR,    CmpStr,    CMP,    STR, STR, NUM )
#define X_OPS_BINARY( X )\
	X_OPS_BINARY_NUM_C( X )\
	X_OPS_BINARY_NUM_MUT_C( X )\
	X_OPS_BINARY_NO_C( X )\
	X_OPS_BINARY_STR_C( X )
/*ALL*************************************************************************/
#define X_OPS( X )\
	X_OPS_EXCLUDE( X )\
	X_OPS_CORE( X )\
	X_OPS_UNARY( X )\
	X_OPS_POST( X )\
	X_OPS_BINARY( X )
/*INITS***********************************************************************/
#define X_OP_ENUMS( OP, FN, TK, LHS, RHS, OUT ) OP_##OP,
#define X_OP_NAMES( OP, FN, TK, LHS, RHS, OUT ) ( u8* )#OP,
#define X_OP_MUTS( OP, FN, TK, LHS, RHS, OUT ) [ OP_##OP ] = 1,
#define X_OP_UNARY_INIT( OP, FN, TK, LHS, RHS, OUT )\
	[ EXPR_##LHS ][ TK_##TK ] = { .code = OP_##OP, .type = EXPR_##OUT },
#define X_OP_POST_INIT( OP, FN, TK, LHS, RHS, OUT )\
	[ EXPR_##LHS ][ TK_##TK ] = { .code = OP_##OP, .type = EXPR_##OUT },
#define X_OP_BINARY_INIT( OP, FN, TK, LHS, RHS, OUT )\
	[ EXPR_##LHS ][ EXPR_##RHS ][ TK_##TK ] = { .code = OP_##OP, .type = EXPR_##OUT },
#define X_OP_VM_CORE_CASE( OP, FN, TK, LHS, RHS, OUT ) case OP_##OP:{ Vm##FN( vm, i ); continue; }
#define X_OP_VM_UNARY_CASE( OP, FN, TK, LHS, RHS, OUT )\
	case OP_##OP:{ Vm##FN( &vm->regs[ i->a ], &vm->regs[ i->b ] ); continue; }
#define X_OP_VM_POST_CASE X_OP_VM_UNARY_CASE
#define X_OP_VM_BINARY_CASE( OP, FN, TK, LHS, RHS, OUT )\
	case OP_##OP:{ Vm##FN( &vm->regs[ i->a ], &vm->regs[ i->b ], &vm->regs[ i->c ] ); continue; }
typedef enum OpCode { X_OPS( X_OP_ENUMS ) OP_COUNT } OpCode;
typedef struct Op {
	u8 code; /* OpCode */
	u8 type; /* ExprType */
} Op;
#endif

#ifdef IMPL
Op* OpGetUnary( ExprType rhs_type, TkType tk_type ){
	static Op unary_ops[ EXPR_COUNT ][ TK_COUNT ] = { X_OPS_UNARY( X_OP_UNARY_INIT ) };
	return &unary_ops[ rhs_type ][ tk_type ];
}

Op* OpGetPost( ExprType lhs_type, TkType tk_type ){
	static Op post_ops[ EXPR_COUNT ][ TK_COUNT ] = { X_OPS_POST( X_OP_POST_INIT ) };
	return &post_ops[ lhs_type ][ tk_type ];
}

Op* OpGetBinary( ExprType lhs_type, ExprType rhs_type, TkType tk_type ){
	static Op binary_ops[ EXPR_COUNT ][ EXPR_COUNT ][ TK_COUNT ] = { X_OPS_BINARY( X_OP_BINARY_INIT ) };
	return &binary_ops[ lhs_type ][ rhs_type ][ tk_type ];
}

u8 OpIsMutative( OpCode code ){
	static u8 mut_ops[ OP_COUNT ] = {
		X_OPS_UNARY_NUM_MUT_C( X_OP_MUTS )
		X_OPS_POST_NUM_MUT_C( X_OP_MUTS )
		X_OPS_BINARY_NUM_MUT_C( X_OP_MUTS )
	};
	return mut_ops[ code ];
}

u8* OpGetName( OpCode op ){
	static u8* names[ ] = { X_OPS( X_OP_NAMES ) };
	return names[ op ];
}
#endif