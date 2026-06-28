#ifdef TYPES
typedef enum ValueType {
	VALUE_NULL,
	VALUE_NUM,
	VALUE_STR,
} ValueType;

typedef struct Value { /* 4 bytes of waste */
	ValueType type;
	union {
		f64 num;
		InternIdx str;
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
#endif