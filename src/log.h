#ifdef TYPES
#define X_LOGS( X )\
	X( LEX_BADCHAR,		"unexpected char '%c'" )\
	X( LEX_BADASSIGN,	": = assign, not =" )\
	X( LEX_BADLOOP,		";; = loop" )\
	X( LEX_BADIF,		"?( = if, ?""?( = elif, ?? = else" ) /* trigraphs -.- */\
	X( LEX_BADSTR,		"unterminated string" )\
	X( PARSE_BADPRE,	"bad expr prefix: %s for token type: %s" )\
	X( PARSE_BADINF,	"bad expr infix: %s for token type: %s" )\
	X( PARSE_BADPOST,	"bad expr postfix: %s for token type: %s" )\
	X( PARSE_EXPECT,	"expected: %s, got: %s" )\
	X( CMP_BADUNARY,	"cannot use %s on %s" )\
	X( CMP_BADPOST,		"cannot use %s on %s" )\
	X( CMP_BADBINARY,	"cannot use %s on %s and %s" )\
	X( CMP_BADID,		"undefined reference: %s" )\
	X( CMP_BADBRK,      "cannot break outside a loop" )\
	X( CMP_BADCONT,     "cannot continue outside a loop" )\
	X( CMP_BADRET,      "cannot return outside a function" )\
	X( CMP_BADCALL,     "call to undefined reference: %s" )
#define X_LOG_ENUMS( ENUM, FMT ) ENUM,
#define X_LOG_FMTS( ENUM, FMT ) ( u8* )FMT,
typedef u32 LogIdx, LogOffset;
typedef enum LogType { X_LOGS( X_LOG_ENUMS ) } LogType;
typedef struct Logs {
	Srcs* srcs;
	Aob aob;
	LogOffset* data;
	u32 len;
	u32 cap;
} Logs;
#endif

#ifdef IMPL
void LogInit( Logs* logs, Srcs* srcs ){
	logs->srcs = srcs;
	AobInit( &logs->aob, LOG_AOB_CAP );
	logs->data = MemAlloc( sizeof( LogOffset ), LOG_VEC_CAP );
	logs->len = 0;
	logs->cap = LOG_AOB_CAP;
}

void LogFree( Logs* logs ){
	MemFree( logs->data );
	AobFree( &logs->aob );
}

static u8* LogGetFmt( LogType err ){
	static u8* fmts[ ] = { X_LOGS( X_LOG_FMTS ) };
	return fmts[ err ];
}

static LogOffset LogCopy( Logs* logs, u8* src, u32 total ){
	LogOffset offset = AobPush( &logs->aob, total + 1 );
	u8* dst = AobGet( &logs->aob, offset );
	memcpy( dst, src, total );
	dst[ total ] = '\0';
	return offset;
}

static void LogGrow( Logs* logs ){
	logs->cap <<= 1;
	logs->data = MemRealloc( logs->data, sizeof( LogOffset ), logs->cap );
}

static LogIdx LogPush( Logs* logs ){
	if( logs->len >= logs->cap ) LogGrow( logs );
	return logs->len++;
}

static LogOffset* LogGet( Logs* logs, LogIdx idx ){
	return &logs->data[ idx ];
}

static LogIdx LogWrite( Logs* logs, LogOffset offset ){
	LogIdx idx = LogPush( logs );
	*LogGet( logs, idx ) = offset;
	return idx;
}

LogIdx Log( Logs* logs, SrcPos* pos, LogType type, ... ){
	u8 buf[ LOG_BUF_CAP ];
	u8* path = SrcGetPath( logs->srcs, pos->src );
	int len1 = snprintf( ( x8* )buf, LOG_BUF_CAP, "%s:%d:%d: ", path, pos->ln, pos->col );
	if( len1 < 0 || len1 >= LOG_BUF_CAP ) Halt( ERR_LOGBUF );
	va_list args;
	va_start( args, type );
	int len2 = vsnprintf( ( x8* )buf + len1, LOG_BUF_CAP - len1, ( x8* )LogGetFmt( type ), args );
	u32 total = len1 + len2;
	if( len2 < 0 || total >= LOG_BUF_CAP ) Halt( ERR_LOGBUF );
	va_end( args );
	LogOffset offset = LogCopy( logs, buf, total );
	return LogWrite( logs, offset );
}

u8 LogDump( Logs* logs ){
	if( !logs->len ) return 0;
	for( u32 i = 0; i < logs->len; i++ ){
		u8* msg = AobGet( &logs->aob, logs->data[ i ] );
		fprintf( stderr, "%s\n", msg );
	}
	return 1;
}
#endif