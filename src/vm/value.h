#ifndef VM_VALUE_H
#define VM_VALUE_H

#include "../base/typedefs.h"
#include "../base/intern/intern.h"

typedef enum ValueType {
	VALUE_NULL,
	VALUE_NUM,
	VALUE_STR,
} ValueType;

typedef struct Value { /* 4 bytes of waste */
	ValueType type;
	union {
		f64 num;
		InternId str;
	};
} Value;

#endif