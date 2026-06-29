#ifdef TYPES
#define X_NATIVES( X )\
	X( PRINT, Print, 1 )\
	X( DUMP, Dump, 0 )\
	X( TYPE, Type, 1 )

#define X_NATIVE_ENUMS( ENUM, NAME, NARGS ) NATIVE_##ENUM,
#define X_NATIVE_INIT( ENUM, NAME, NARGS ) [ NATIVE_##ENUM ] = { .name = ( u8* )#NAME, .nargs = NARGS },
#define X_NATIVE_CASE( ENUM, NAME, NARGS ) case NATIVE_##ENUM:{ Vm##NAME( vm, i ); return; }

typedef enum NativeType { X_NATIVES( X_NATIVE_ENUMS ) NATIVE_COUNT } NativeType;
typedef struct Native {
	u8* name;
	u8 nargs;
} Native;
#endif

#ifdef IMPL
void NativeInit( App* app ){
	static Native natives[ ] = { X_NATIVES( X_NATIVE_INIT ) };
	Interns* interns = &app->interns;
	Fns* fns = &app->fns;

	for( u32 i = 0; i < NATIVE_COUNT; i++ ){
		Native* native = &natives[ i ];
		InternIdx intern = InternPut( interns, native->name, HASH_ID );
		FnPushNative( fns, intern, i, native->nargs );
	}		
}
#endif