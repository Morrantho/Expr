#include "log.h"

static LogList* LogGet( ){
	static LogList log;
	return &log;
}

static x8* LogGetFmt( LogType type ){
	static x8* fmts[ ] = { X_LOGS( X_LOG_FMTS ) };
	return fmts[ type ];
}

static LogLvl LogGetLvl( LogType type ){
	static LogLvl lvls[ ] = { X_LOGS( X_LOG_LVLS ) };
	return lvls[ type ];
}

void LogInit( u32 store_cap, u32 entry_cap ){
	LogList* log = LogGet( );
	log->store = MemAlloc( store_cap, sizeof( u8 ) );
	log->store_len = 0;
	log->store_cap = store_cap;

	log->entries = MemAlloc( entry_cap, sizeof( LogEntry ) );
	log->entry_cap = entry_cap;
	log->entry_len = 0;
}

void LogReset( ){
	LogList* log = LogGet( );
	log->entry_len = log->store_len = log->fatal = 0;
}

static void LogStoreGrow( LogList* log, u32 total ){
	while( log->store_cap < total ) log->store_cap <<= 1;
	log->store = MemRealloc( log->store, sizeof( u8 ), log->store_cap );
}

static u32 LogStorePush( LogList* log, x8* src, u32 len ){
	u32 off = log->store_len;
	u32 total = off + len + 1;
	if( total > log->store_cap ) LogStoreGrow( log, total );
	x8* dst = log->store + off;
	memcpy( dst, src, len );
	dst[ len ] = '\0';
	log->store_len = total;
	return off;
}

static void LogEntriesGrow( LogList* log ){
	log->entry_cap <<= 1;
	log->entries = MemRealloc( log->entries, sizeof( LogEntry ), log->entry_cap );
}

static void LogEntryPush( LogList* log, u32 store_offset, Src* src, LogType type ){
	if( log->entry_len >= log->entry_cap ) LogEntriesGrow( log );
	u32 entry_id = log->entry_len++;
	LogEntry* entry = &log->entries[ entry_id ];
	entry->ln = src->ln;
	entry->col = src->col;
	entry->path = src->path;
	entry->type = type;
	entry->store_offset = store_offset;
}

void Log( Src* src, LogType type, ... ){
	LogList* log = LogGet( );
	x8 buf[ LOG_BUF_MAX ];
	x8* fmt = LogGetFmt( type );
	va_list args;
	va_start( args, type );
	x32 len = vsnprintf( buf, sizeof( buf ), fmt, args );
	va_end( args );
	if( len < 0 ){ Throw( ERR_LOGBUF, fmt ); return; }
	if( len >= LOG_BUF_MAX ) len = LOG_BUF_MAX-1; /* Trunc it */
	u32 off = LogStorePush( log, buf, ( u32 )len );
	LogEntryPush( log, off, src, type );
	log->fatal |= LogGetLvl( type ) == LOG_FATAL; /* so we dont overwrite it if previously 1 */
}

u8 LogIsFatal( ){ return LogGet( )->fatal; }

void LogFlush( ){
	LogList* log = LogGet( );
	for( u32 i = 0; i < log->entry_len; i++ ){
		LogEntry* e = &log->entries[ i ];
		x8* msg = log->store + e->store_offset;
		fprintf( stderr, "%s:%u:%u: %s", e->path, e->ln, e->col, msg );
	}
	log->entry_len = log->store_len = 0;
}

void LogFree( ){
	LogList* log = LogGet( );
	MemFree( log->entries );
	MemFree( log->store );
	*log = ( LogList){ 0 };
}