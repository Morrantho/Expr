#ifndef FUNC_H
#define FUNC_H

#include "../base/typedefs.h"
#include "inst.h"

#define FUNC_NONE UINT16_MAX

typedef u16 FuncId;

typedef struct Func {
	u32 start;	/* insts start index */
	u32 end;	/* insts end index */
	u8 nargs;
	u8 nregs;
	ExprType ret_type;
} Func;

typedef struct Funcs {
	Func* data;
	u32 len;
	u32 cap;
} Funcs;

void FuncInit( Funcs* funcs );
FuncId FuncPush( Funcs* funcs );
Func* FuncGet( Funcs* funcs, FuncId id );
void FuncFree( Funcs* funcs );

#endif