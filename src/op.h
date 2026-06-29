#ifdef TYPES
/*EXCLUDE*********************************************************************/
#define X_OPS_EXCLUDE( X )\
	X( ERR,		_,			_,		0	)\
	X( HALT,	_,			_,		0	)
/*CORE************************************************************************/
#define X_OPS_CORE( X )\
	X( LOADC,	LoadConst,	_,		0	)\
	X( JMP,		Jmp,		_,		0	)\
	X( JZ,		Jz,			_,		0	)\
	X( JNZ,		Jnz,		_,		0	)\
	X( MOV,		Mov,		_,		0	)\
	X( RET,		Return,		_,		0	)\
	X( CALL,	Call,		_,		0	)
/*UNARY***********************************************************************/
#define X_OPS_UNARY( X )\
	X( NOT,		Not,		NOT,	0	)\
	X( NEG,		Neg,		SUB,	0	)\
	X( BNOT,	Bnot,		BNOT,	0	)\
	X( ROUND,	Round,		ROUND,	0	)\
	X( CEIL,	Ceil,		CEIL,	0	)\
	X( FLOOR,	Floor,		FLOOR,	0	)\
	X( PREINC,  PreInc, 	INC,	1	)\
	X( PREDEC,	PreDec, 	DEC,	1	)
/*POSTFIX*********************************************************************/
#define X_OPS_POST( X )\
	X( POSTINC,	PostInc,	INC,	1	)\
	X( POSTDEC, PostDec,	DEC,	1	)
/*BINARY**********************************************************************/
#define X_OPS_BINARY( X )\
	X( NOTEQ,	NotEq,		NOTEQ,	0	)\
	X( MOD,		Mod,		MOD,	0	)\
	X( BAND,	Band,		BAND,	0	)\
	X( AND,		And,		AND,	0	)\
	X( MUL,		Mul,		MUL,	0	)\
	X( ADD,		Add,		ADD,	0	)\
	X( SUB,		Sub,		SUB,	0	)\
	X( DIV,		Div,		DIV,	0	)\
	X( LT,		Lt,			LT,		0	)\
	X( LSH,		Lsh,		LSH,	0	)\
	X( LTE,		Lte,		LTE,	0	)\
	X( EQ,		Eq,			EQ,		0	)\
	X( GT,		Gt,			GT,		0	)\
	X( RSH,		Rsh,		RSH,	0	)\
	X( GTE,		Gte,		GTE,	0	)\
	X( BXOR,	Bxor,		BXOR,	0	)\
	X( BOR,		Bor,		BOR,	0	)\
	X( OR,		Or,			OR,		0	)\
	X( POW,		Pow,		POW,	0	)\
	X( MODEQ,	ModEq,		MODEQ,	1	)\
	X( BANDEQ,	BandEq,		BANDEQ,	1	)\
	X( MULEQ,	MulEq,		MULEQ,	1	)\
	X( ADDEQ,	AddEq,		ADDEQ,	1	)\
	X( SUBEQ,	SubEq,		SUBEQ,	1	)\
	X( DIVEQ,	DivEq,		DIVEQ,	1	)\
	X( LSHEQ,	LshEq,		LSHEQ,	1	)\
	X( RSHEQ,	RshEq,		RSHEQ,	1	)\
	X( BXOREQ,	BxorEq,		BXOREQ,	1	)\
	X( BOREQ,	BorEq,		BOREQ,	1	)
/*ALL*************************************************************************/
#define X_OPS( X )\
	X_OPS_EXCLUDE( X )\
	X_OPS_CORE( X )\
	X_OPS_UNARY( X )\
	X_OPS_POST( X )\
	X_OPS_BINARY( X )
/*INITS***********************************************************************/
#define X_OP_ENUMS( OP, FN, TK, MUT ) OP_##OP,
#define X_OP_NAMES( OP, FN, TK, MUT ) ( u8* )#OP,
#define X_OP_INIT( OP, FN, TK, MUT ) [ TK_##TK ] = { .code = OP_##OP, .mut = MUT },
#define X_OP_VM_CORE_CASE( OP, FN, TK, MUT ) case OP_##OP:{ Vm##FN( vm, i ); continue; }
#define X_OP_VM_UNARY_CASE( OP, FN, TK, MUT )\
	case OP_##OP:{ Vm##FN( &vm->regs[ i->a ], &vm->regs[ i->b ] ); continue; }
#define X_OP_VM_POST_CASE X_OP_VM_UNARY_CASE
#define X_OP_VM_BINARY_CASE( OP, FN, TK, MUT )\
	case OP_##OP:{ Vm##FN( &vm->regs[ i->a ], &vm->regs[ i->b ], &vm->regs[ i->c ] ); continue; }
typedef enum OpCode { X_OPS( X_OP_ENUMS ) OP_COUNT } OpCode;
typedef struct Op {
	u8 code;
	u8 mut;
} Op;
#endif

#ifdef IMPL
Op* OpGetUnary( TkType tk_type ){
	static Op unary_ops[ TK_COUNT ] = { X_OPS_UNARY( X_OP_INIT ) };
	return &unary_ops[ tk_type ];
}

Op* OpGetPost( TkType tk_type ){
	static Op post_ops[ TK_COUNT ] = { X_OPS_POST( X_OP_INIT ) };
	return &post_ops[ tk_type ];
}

Op* OpGetBinary( TkType tk_type ){
	static Op binary_ops[ TK_COUNT ] = { X_OPS_BINARY( X_OP_INIT ) };
	return &binary_ops[ tk_type ];
}

u8* OpGetName( OpCode op ){
	static u8* names[ ] = { X_OPS( X_OP_NAMES ) };
	return names[ op ];
}
#endif