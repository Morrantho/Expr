#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include "cfg.h"
#include "src.h"

#define X_LOGS( X )\
	X( WARN, LEX_BADCHAR,   "Unexpected char '%c'\n" )\
	X( WARN, LEX_BADASSIGN, "Use ':' for assignments, not '='\n" )

#define X_LOG_LVLS( LEVEL, TYPE, FMT ) LOG_##LEVEL,
#define X_LOG_TYPES( LEVEL, TYPE, FMT ) TYPE,
#define X_LOG_FMTS( LEVEL, TYPE, FMT ) FMT,
typedef enum LogLvl { LOG_WARN, LOG_FATAL } LogLvl;
typedef enum LogType { X_LOGS( X_LOG_TYPES ) } LogType;

typedef struct LogEntry {
	u8* path;
	u32 ln, col;
	LogType type;
	u32 store_offset;
} LogEntry;

typedef struct LogList {
	x8* store; /* stores the entire post-formatted strings here. */
	u32 store_len;
	u32 store_cap;

	LogEntry* entries;
	u32 entry_len;
	u32 entry_cap;

	u8 fatal;
} LogList;

void LogInit( u32 store_cap, u32 entry_cap );
void LogReset( );
void Log( Src* src, LogType type, ... );
void LogFlush( );
u8 LogIsFatal( );
void LogFree( );

#endif