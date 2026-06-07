#include "log.h"

static LogLvl LogGetLvl( LogMsgType type ){
	static LogLvl lvls[ ] = { X_LOGS( X_LOG_LVL_INIT ) };
	return lvls[ type ];
}

static u8* LogGetFmt( LogMsgType type ){
	static u8* fmts[ ] = { X_LOGS( X_LOG_FMT_INIT ) };
	return fmts[ type ];
}

static u8* LogGetName( LogLvl lvl ){
	static u8* names[ ] = { X_LOG_TYPES( X_LOG_NAME_INIT ) };
	return names[ lvl ];
}

static u8* LogGetCol( LogLvl lvl ){
	static u8* cols[ ] = { X_LOG_TYPES( X_LOG_COL_INIT ) };
	return cols[ lvl ];
}

void LogInit( Logs* logs, Srcs* sources ){
	logs->sources = sources;
	logs->entries = MemAlloc( sizeof( LogEntry ), LOG_CAP );
	AobInit( &logs->aob, LOG_AOB_CAP );
	logs->len = 0;
	logs->cap = LOG_CAP;
}

void LogReset( Logs* logs ){
	AobReset( &logs->aob );
	logs->len = 0;
}

static Offset LogMsgPush( Logs* logs, u8* src, u32 len ){
	Offset msg_off = AobPush( &logs->aob, len + 1 );
	u8* dst = AobGet( &logs->aob, msg_off );
	memcpy( dst, src, len );
	dst[ len ] = '\0';
	return msg_off;
}

static void LogEntriesGrow( Logs* logs ){
	logs->cap <<= 1;
	logs->entries = MemRealloc( logs->entries, sizeof( LogEntry ), logs->cap );
}

static void LogEntryPush( Logs* logs, Offset msg, LogPos* pos, LogMsgType msg_type ){
	if( logs->len >= logs->cap ) LogEntriesGrow( logs );
	LogEntry* entry = &logs->entries[ logs->len++ ];
	entry->msg_type = msg_type;
	entry->pos = *pos; /* Copy the whole thing */
	entry->msg = msg;
}

void Log( Logs* logs, LogPos* pos, LogMsgType type, ... ){
	u8 buf[ LOG_BUF_CAP ];
	u8* fmt = LogGetFmt( type );
	va_list args;
	va_start( args, type );
	x32 len = vsnprintf( ( x8* )buf, sizeof( buf ), ( x8* )fmt, args );
	va_end( args );
	if( len < 0 ){ Throw( ERR_LOGBUF, fmt ); return; }
	if( ( u32 )len >= LOG_BUF_CAP ) len = LOG_BUF_CAP-1; /* Trunc it */
	Offset msg = LogMsgPush( logs, buf, ( u32 )len );
	LogEntryPush( logs, msg, pos, type );
}

u8 LogDump( Logs* logs ){ /* nonzero = fatal */
	u8 fatal = 0;
	for( u32 i = 0; i < logs->len; i++ ){
		LogEntry* e = &logs->entries[ i ];
		LogLvl lvl = LogGetLvl( e->msg_type );
		u8* name = LogGetName( lvl );
		u8* col = LogGetCol( lvl );
		u8* path = SrcGetPath( logs->sources, e->pos.src );
		u8* msg = AobGet( &logs->aob, e->msg );
		fprintf( stderr, "%s%s: %s:%u:%u: %s\033[0m\n", col, name, path, e->pos.ln, e->pos.col, msg );
		fatal |= lvl == LOG_FATAL;
	}
	AobReset( &logs->aob );
	logs->len = 0;
	return fatal;
}

void LogFree( Logs* logs ){
	AobFree( &logs->aob );
	MemFree( logs->entries );
	*logs = ( Logs ){ 0 };
}