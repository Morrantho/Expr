#ifdef TYPES
#define X_TKS( X ) /* prefix, infix, and postfix point to a denotation type enum */\
	/* ENUM     PREC    ASSOC  PREFIX  INFIX  POSTFIX SEQ  */\
	X( EOS,     NONE,   NONE,  ERR,    ERR,   ERR,  "EOS"   )\
	X( ERR,     NONE,   NONE,  ERR,    ERR,   ERR,  "ERR"   )\
	X( NOT,     UNARY,  RIGHT, PRE,    ERR,   ERR,  "!"     )\
	X( NOTEQ,   EQUAL,  LEFT,  ERR,    INF,   ERR,  "!="    )\
	X( STR,     NONE,   NONE,  STR,    ERR,   ERR,  "STR"   )\
	X( MOD,     FACTOR, LEFT,  ERR,    INF,   ERR,  "%"     )\
	X( ROUND,   UNARY,  RIGHT, PRE,    ERR,   ERR,  "%%"    )\
	X( MODEQ,   ASSIGN, RIGHT, ERR,    INF,   ERR,  "%="    )\
	X( BAND,    BAND,   LEFT,  ERR,    INF,   ERR,  "&"     )\
	X( AND,     AND,    LEFT,  ERR,    INF,   ERR,  "&&"    )\
	X( BANDEQ,  ASSIGN, RIGHT, ERR,    INF,   ERR,  "&="    )\
	X( LP,      NONE,   NONE,  GRP,    ERR,   CALL, "("     )\
	X( RP,      NONE,   NONE,  ERR,    ERR,   ERR,  ")"     )\
	X( FNCLOSE, NONE,   NONE,  ERR,    ERR,   ERR,  ")>"    )\
	X( MUL,     FACTOR, LEFT,  ERR,    INF,   ERR,  "*"     )\
	X( CEIL,    UNARY,  RIGHT, PRE,    ERR,   ERR,  "**"    )\
	X( MULEQ,   ASSIGN, RIGHT, ERR,    INF,   ERR,  "*="    )\
	X( ADD,     TERM,   LEFT,  NOPPRE, INF,   ERR,  "+"     )\
	X( INC,     UNARY,  LEFT,  PRE,    ERR,   POST,  "++"   )\
	X( ADDEQ,   ASSIGN, RIGHT, ERR,    INF,   ERR,  "+="    )\
	X( COMMA,   NONE,   NONE,  ERR,    ERR,   ERR,  ","     )\
	X( SUB,     TERM,   LEFT,  PRE,    INF,   ERR,  "-"     )\
	X( DEC,     UNARY,  LEFT,  PRE,    ERR,   POST, "--"    )\
	X( SUBEQ,   ASSIGN, RIGHT, ERR,    INF,   ERR,  "-="    )\
	X( MEMBER,  NONE,   NONE,  ERR,    ERR,   ERR,  "."     )\
	X( DIV,     FACTOR, LEFT,  ERR,    INF,   ERR,  "/"     )\
	X( FLOOR,   UNARY,  RIGHT, PRE,    ERR,   ERR,  "//"    )\
	X( DIVEQ,   ASSIGN, RIGHT, ERR,    INF,   ERR,  "/="    )\
	X( NUM,     NONE,   NONE,  NUM,    ERR,   ERR,  "NUM"   )\
	X( ASSIGN,  ASSIGN, RIGHT, ERR,    ERR,   ERR,  ":"     )\
	X( END,     NONE,   NONE,  ERR,    ERR,   ERR,  "::"    )\
	X( LOOP,    NONE,   NONE,  ERR,    ERR,   ERR,  ";;"    )\
	X( LT,      REL,    LEFT,  ERR,    INF,   ERR,  "<"     )\
	X( LSH,     SHIFT,  LEFT,  ERR,    INF,   ERR,  "<<"    )\
	X( LTE,     REL,    LEFT,  ERR,    INF,   ERR,  "<="    )\
	X( FNOPEN,  NONE,   NONE,  ERR,    ERR,   ERR,  "<("    )\
	X( LSHEQ,   ASSIGN, RIGHT, ERR,    INF,   ERR,  "<<="   )\
	X( BREAK,   NONE,   NONE,  ERR,    ERR,   ERR,  "<=="   )\
	X( CMP,     EQUAL,  LEFT,  ERR,    INF,   ERR,  "=="    )\
	X( CONT,    NONE,   NONE,  ERR,    ERR,   ERR,  "==>"   )\
	X( GT,      REL,    LEFT,  ERR,    INF,   ERR,  ">"     )\
	X( RSH,     SHIFT,  LEFT,  ERR,    INF,   ERR,  ">>"    )\
	X( GTE,     REL,    LEFT,  ERR,    INF,   ERR,  ">="    )\
	X( RSHEQ,   ASSIGN, RIGHT, ERR,    INF,   ERR,  ">>="   )\
	X( IF,      NONE,   NONE,  ERR,    ERR,   ERR,  "?("    )\
	X( ELSE,    NONE,   NONE,  ERR,    ERR,   ERR,  "??"    )\
	X( ELIF,    NONE,   NONE,  ERR,    ERR,   ERR,  "?""?(" )\
	X( RET,     NONE,   NONE,  ERR,    ERR,   ERR,  "<-"    )\
	X( ID,      NONE,   NONE,  ID,     ERR,   ERR,  "ID"    )\
	X( BXOR,    BXOR,   LEFT,  ERR,    INF,   ERR,  "^"     )\
	X( POW,     POW,    LEFT,  ERR,    INF,   ERR,  "^^"    )\
	X( BXOREQ,  ASSIGN, RIGHT, ERR,    INF,   ERR,  "^="    )\
	X( BOR,     BOR,    LEFT,  ERR,    INF,   ERR,  "|"     )\
	X( OR,      OR,     LEFT,  ERR,    INF,   ERR,  "||"    )\
	X( BOREQ,   ASSIGN, RIGHT, ERR,    INF,   ERR,  "|="    )\
	X( BNOT,    UNARY,  RIGHT, PRE,    ERR,   ERR,  "~"     )
#define X_TK_ENUMS( ENUM, PREC, ASSOC, PRE, INF, POST, SEQ ) TK_##ENUM,
#define X_TK_STRS( ENUM, PREC, ASSOC, PRE, INF, POST, SEQ ) ( u8* )SEQ,
typedef enum TkType { X_TKS( X_TK_ENUMS ) TK_COUNT } TkType;
typedef struct Tk {
	TkType type;
	SrcPos pos;
	union {
		f64 num;
		InternIdx intern;
	};
} Tk;
#endif

#ifdef IMPL
u8* TkGetName( TkType type ){
	static u8* types[ ] = { X_TKS( X_TK_STRS ) };
	return types[ type ];
}
#endif