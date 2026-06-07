#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include "cfg.h"
#include "src.h"

#define X_LOG_TYPES( X )\
	X( WARN,  "warn",  "\033[33m" )\
	X( FATAL, "fatal", "\033[1;31m" )

#define X_LOGS( X )\
	X( WARN, LEX_BADCHAR,   "unexpected char '%c'" )\
	X( WARN, LEX_BADASSIGN, "use ':' for assignments, not '='" )

#define X_LOG_LVL_ENUM( LEVEL, NAME, COL ) LOG_##LEVEL,
#define X_LOG_MSG_TYPE_ENUM( LEVEL, TYPE, FMT ) TYPE,
#define X_LOG_LVL_INIT( LEVEL, TYPE, FMT ) LOG_##LEVEL,
#define X_LOG_FMT_INIT( LEVEL, TYPE, FMT ) ( u8* )FMT,
#define X_LOG_NAME_INIT( LEVEL, NAME, COL ) ( u8* )NAME,
#define X_LOG_COL_INIT( LEVEL, NAME, COL ) ( u8* )COL,

typedef enum LogLvl { X_LOG_TYPES( X_LOG_LVL_ENUM ) } LogLvl;
typedef enum LogMsgType { X_LOGS( X_LOG_MSG_TYPE_ENUM ) } LogMsgType;

typedef struct LogPos {
	SrcId src;			/* The source file it came from */
	u32 ln;				/* The source line */
	u32 col;			/* The source column */
} LogPos;

typedef struct LogEntry {
	LogMsgType msg_type;/* Used for fmt and lvl lookup on Flushes */
	LogPos pos;			/* Where it occured */
	Offset msg;			/* The partially formatted message */
} LogEntry;

typedef struct LogList {
	SrcList* sources;	/* Shared from App. Needed for source lookups. */
	LogEntry* entries;	/* Metadata for later lookups. */
	Aob msgs;			/* formatted messages, not full entries. */
	u32 len;			/* Entry length */
	u32 cap;			/* Entry cap */
} LogList;

void LogInit( LogList* log, SrcList* sources );
void LogReset( LogList* log );
void Log( LogList* log, LogPos* pos, LogMsgType type, ... );
u8 LogDump( LogList* log ); /* nonzero = fatal */
void LogFree( LogList* log );

#endif