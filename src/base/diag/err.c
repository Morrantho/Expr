#include "err.h"

void Throw( ErrType err, ... ){
	static u8* fmts[ ] = { X_ERRS( X_ERR_FMTS ) };
	va_list args;
	va_start( args, err );
	vfprintf( stderr, ( x8* )fmts[ err ], args );
	va_end( args );
	exit( EXIT_FAILURE );
}