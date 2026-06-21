#ifdef TYPES
#define X_PRECS( X )\
	X( NONE )   /*  */\
	X( COMMA )  /* , */\
	X( ASSIGN ) /* = += -= *= /= %= <<= >>= &= |= */\
	X( TERN )   /* ?: */\
	X( OR )     /* || */\
	X( AND )	/* && */\
	X( BOR )    /* | */\
	X( BXOR )   /* ^ */\
	X( BAND )   /* & */\
	X( EQUAL )  /* == != */\
	X( REL )    /* < <= > >= */\
	X( SHIFT )  /* << >> */\
	X( TERM )   /* + - */\
	X( FACTOR ) /* * / */\
	X( UNARY )  /* + - ! ~ * // ** %% */\
	X( POW )    /* ^^ */\
	X( POST )   /* [ ] ( ) . ++ -- */
#define X_DENOS( X )\
	X( ERR )\
	X( NOPPRE )\
	X( GRP )\
	X( PRE )\
	X( POST )\
	X( INF )\
	X( NUM )\
	X( STR )\
	X( ID )
	// X( FN )
	// X( CALL )
	// X( MEMB )
#define X_EXPRS( X )\
	X( ERR,		"error" )\
	X( VOID,	"void" )\
	X( NUM,		"number" )\
	X( STR,		"string" )\
	X( ID,		"id" )
	// X( FN,		"function" )
#define X_PREC_ENUMS( ENUM ) PREC_##ENUM,
#define X_DENO_ENUMS( ENUM ) DENO_##ENUM,
#define X_EXPR_ENUMS( ENUM, STR ) EXPR_##ENUM,
#define X_DENO_NAMES( ENUM ) ( u8* )#ENUM,
#define X_EXPR_STRS( ENUM, STR ) ( u8* )STR,
#define X_TK_PRECS( TK, PREC, ASSOC, PRE, INF, POST, SEQ ) PREC_##PREC,
#define X_TK_ASSOCS( TK, PREC, ASSOC, PRE, INF, POST, SEQ ) ASSOC_##ASSOC,
#define X_DENO_PRES( TK, PREC, ASSOC, PRE, INF, POST, SEQ ) [ PARSEPOS_PRE ][ TK_##TK ]  = DENO_##PRE,
#define X_DENO_INFS( TK, PREC, ASSOC, PRE, INF, POST, SEQ ) [ PARSEPOS_INF ][ TK_##TK ]  = DENO_##INF,
#define X_DENO_POSTS( TK, PREC, ASSOC, PRE, INF, POST, SEQ ) [ PARSEPOS_POST ][ TK_##TK ] = DENO_##POST,
typedef enum ParsePos {
	PARSEPOS_PRE,
	PARSEPOS_INF,
	PARSEPOS_POST,
	PARSEPOS_COUNT
} ParsePos;
typedef enum Deno { X_DENOS( X_DENO_ENUMS ) DENO_COUNT } Deno;
typedef enum Prec { X_PRECS( X_PREC_ENUMS ) PREC_COUNT } Prec;
typedef enum Assoc {
	ASSOC_NONE = 0,
	ASSOC_LEFT = 1,
	ASSOC_RIGHT,
} Assoc;
typedef enum ExprType { X_EXPRS( X_EXPR_ENUMS )  EXPR_COUNT } ExprType;
typedef struct Expr { /* 8 bytes max. If we need more metadata, use u8s for these. */
	ExprType type;
	union {
		InternIdx intern;
		u32 reg;
	};
} Expr;
#endif

#ifdef IMPL
Prec PrecGet( TkType type ){
	static u8 precs[ ] = { X_TKS( X_TK_PRECS ) };
	return ( Prec )precs[ type ];
}

Assoc AssocGet( TkType type ){
	static u8 assocs[ ] = { X_TKS( X_TK_ASSOCS ) };
	return ( Assoc )assocs[ type ];
}

Deno DenoGet( ParsePos pos, TkType type ){
	static u8 denos[ PARSEPOS_COUNT ][ TK_COUNT ] = {
		X_TKS( X_DENO_PRES )
		X_TKS( X_DENO_INFS )
		X_TKS( X_DENO_POSTS )
	};
	return ( Deno )denos[ pos ][ type ];
}

u8* DenoGetName( Deno deno ){
	static u8* names[ ] = { X_DENOS( X_DENO_NAMES ) };
	return names[ deno ];
}

u8* ExprGetName( ExprType type ){
	static u8* names[ ] = { X_EXPRS( X_EXPR_STRS ) };
	return names[ type ];
}

Expr ExprGen( ExprType type, u32 reg ){
	return ( Expr ){ .type = type, .reg = reg };
}
#endif