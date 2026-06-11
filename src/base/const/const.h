#ifndef CONST_H
#define CONST_H

#include "../typedefs.h"
#include "../cfg.h"
#include "../intern/intern.h"

typedef u32 ConstId;

typedef enum ConstType {
	CONST_NUM,
	CONST_STR,
	CONST_COUNT
} ConstType;

typedef struct Const{
	ConstType type;
	union {
		f64 num;
		InternId str;
	};
} Const;

typedef struct Consts {
	Const* items;
	u32 len;
	u32 cap;
} Consts;

void ConstInit( Consts* consts );
ConstId ConstPutNum( Consts* consts, f64 num );
ConstId ConstPutStr( Consts* consts, InternId str );
void ConstFree( Consts* consts );

#endif