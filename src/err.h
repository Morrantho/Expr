#ifdef TYPES
#define X_ERRS( X )\
	X( NOFILE,	"no file provided" )\
	X( OOM, "failed to allocate block: %d. out of memory.\n" )\
	X( FREE, "tried to free a null pointer\n" )\
	X( BADFILE, "file not found: %s\n" )\
	X( FTELL, "failed to read file size from file: %s\n" )\
	X( FREAD, "failed to read file: %s\n" )\
	X( LOGBUF, "log buffer failed to allocate format string: %s\n" )\
	X( REGALLOC, "ran out of registers. aborting.\n")\
	X( BADFN, "invalid function. aborting.\n" )\
	X( REGOVERFLOW, "max registers reached. aborting." )\
	X( FRAMEOVERFLOW, "max frames reached. aborting.\n" )\
	X( FRAMEUNDERFLOW, "frame underflow. aborting.\n" )\
	X( BADFRAME, "bad frame span. aborting\n" )

#define X_ERR_ENUMS( ENUM, FMT ) ERR_##ENUM, 
#define X_ERR_FMTS( ENUM, FMT ) ( u8* )FMT,
typedef enum ErrType { X_ERRS( X_ERR_ENUMS ) } ErrType;
#endif

#ifdef IMPL
u8* ErrGet( ErrType type ){
	static u8* errs[ ] = { X_ERRS( X_ERR_FMTS ) };
	return errs[ type ];
}

_Noreturn void Halt( ErrType type, ... ){
	u8* fmt = ErrGet( type );
	va_list args;
	va_start( args, type );
	vfprintf( stderr, ( x8* )fmt, args );
	va_end( args );
	fputc( '\n', stderr );
	exit( EXIT_FAILURE );
}
#endif