#ifndef ERR_H
#define ERR_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "../typedefs.h"

#define X_ERR_ENUMS( ENUM, FMT ) ERR_##ENUM,
#define X_ERR_FMTS( ENUM, FMT ) ( u8* )FMT,
#define X_ERRS( X )\
	X( OOM, "failed to allocate block: %d. out of memory.\n" )\
	X( FREE, "tried to free a null pointer\n" )\
	X( BADFILE, "file not found: %s\n" )\
	X( FTELL, "failed to read file size from file: %s\n" )\
	X( FREAD, "failed to read file: %s\n" )\
	X( LOGBUF, "log buffer failed to allocate format string: %s\n" )\
	X( REGALLOC, "ran out of registers. aborting.\n")

typedef enum ErrType { X_ERRS( X_ERR_ENUMS ) ERR_COUNT } ErrType;
void Throw( ErrType err, ... );

#endif