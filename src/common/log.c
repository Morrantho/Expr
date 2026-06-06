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

void LogInit( LogList* log, SrcList* sources ){
	log->sources = sources;
	log->entries = MemAlloc( sizeof( LogEntry ), LOG_CAP );
	AobInit( &log->msgs, LOG_AOB_CAP );
	log->len = 0;
	log->cap = LOG_CAP;
}

void LogReset( LogList* log ){
	AobReset( &log->msgs );
	log->len = 0;
}

static Offset LogMsgPush( LogList* log, u8* src, u32 len ){
	Offset msg_off = AobPush( &log->msgs, len + 1 );
	u8* dst = AobGet( &log->msgs, msg_off );
	memcpy( dst, src, len );
	dst[ len ] = '\0';
	return msg_off;
}

static void LogEntriesGrow( LogList* log ){
	log->cap <<= 1;
	log->entries = MemRealloc( log->entries, sizeof( LogEntry ), log->cap );
}

static void LogEntryPush( LogList* log, Offset msg, LogPos* pos, LogMsgType msg_type ){
	if( log->len >= log->cap ) LogEntriesGrow( log );
	LogEntry* entry = &log->entries[ log->len++ ];
	entry->msg_type = msg_type;
	entry->pos = *pos; /* Copy the whole thing */
	entry->msg = msg;
}

void Log( LogList* log, LogPos* pos, LogMsgType type, ... ){
	u8 buf[ LOG_BUF_CAP ];
	u8* fmt = LogGetFmt( type );
	va_list args;
	va_start( args, type );
	x32 len = vsnprintf( ( x8* )buf, sizeof( buf ), ( x8* )fmt, args );
	va_end( args );
	if( len < 0 ){ Throw( ERR_LOGBUF, fmt ); return; }
	if( ( u32 )len >= LOG_BUF_CAP ) len = LOG_BUF_CAP-1; /* Trunc it */
	Offset msg = LogMsgPush( log, buf, ( u32 )len );
	LogEntryPush( log, msg, pos, type );
}

u8 LogDump( LogList* log ){ /* nonzero = fatal */
	u8 fatal = 0;
	for( u32 i = 0; i < log->len; i++ ){
		LogEntry* e = &log->entries[ i ];
		LogLvl lvl = LogGetLvl( e->msg_type );
		u8* name = LogGetName( lvl );
		u8* col = LogGetCol( lvl );
		u8* path = SrcGetPath( log->sources, e->pos.src );
		u8* msg = AobGet( &log->msgs, e->msg );
		fprintf( stderr, "%s%s: %s:%u:%u: %s\033[0m\n", col, name, path, e->pos.ln, e->pos.col, msg );
		fatal |= lvl == LOG_FATAL;
	}
	AobReset( &log->msgs );
	log->len = 0;
	return fatal;
}

void LogFree( LogList* log ){
	AobFree( &log->msgs );
	MemFree( log->entries );
	*log = ( LogList){ 0 };
}