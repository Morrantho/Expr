#ifndef ERR_H
#define ERR_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "typedefs.h"

#define X_ERR_ENUMS( ENUM, FMT ) ERR_##ENUM,
#define X_ERR_FMTS( ENUM, FMT ) FMT,
#define X_ERRS( X )\
	X( OOM, "Failed To Allocate Block: %d. Out Of Memory.\n" )\
	X( FREE, "Tried To Free A Null Pointer!\n" )\
	X( BADFILE, "File Not Found: %s\n" )\
	X( FTELL, "Failed To Read File Size For File: %s\n" )\
	X( FREAD, "Failed To Read Bytes From File: %s\n" )\
	X( LEXBADCHAR, "Unexpected Char: '%c'\n" )\
	X( LEXASSIGN, "Use : For Assignments\n" )

typedef enum ErrType { X_ERRS( X_ERR_ENUMS ) } ErrType;
void Throw( ErrType err, ... );

#endif