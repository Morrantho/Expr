#ifdef TYPES
#define X_VALUES( X )\
	X( NULL, "null" )\
	X( NUM, "number" )\
	X( STR, "string" )\
	X( FN, "function" )
#define X_VALUE_ENUMS( ENUM, STR ) VALUE_##ENUM,
#define X_VALUE_NAMES( ENUM, STR ) ( u8* )STR,
typedef enum ValueType { X_VALUES( X_VALUE_ENUMS ) } ValueType;
typedef struct Value { /* 4 bytes of waste */
	ValueType type;
	union {
		f64 num;
		InternIdx str;
		u32 fn; /* FnIdx */
	};
} Value;
#endif

#ifdef IMPL
static inline Value ValueNull( ){
	return ( Value ){ .type = VALUE_NULL };
}

static inline Value ValueNum( f64 num ){
	return ( Value ){ .type = VALUE_NUM, .num = num };
}

static inline Value ValueStr( InternIdx str ){
	return ( Value ){ .type = VALUE_STR, .str = str };
}

static inline Value ValueFn( u32 fn ){ /* FnIdx */
	return ( Value ){ .type = VALUE_FN, .fn = fn };
}

static inline u8* ValueGetName( ValueType type ){
	static u8* names[ ] = { X_VALUES( X_VALUE_NAMES ) };
	return names[ type ];
}
#endif