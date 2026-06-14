#ifndef SYM_H
#define SYM_H

#include "../base/intern/intern.h"
#include "expr.h"
#include "func.h"

#define SYM_NONE UINT32_MAX

typedef u32 SymId;

typedef enum SymType {
	SYM_VAR,
	SYM_FUNC
} SymType;

typedef struct Sym {
	InternId name;
	union {
		u8 reg;
		FuncId func;
	};
	u8 expr_type;
	u8 sym_type;
} Sym;

typedef struct Syms {
	Sym* data;
	u32 len;
	u32 cap;
} Syms;

void SymInit( Syms* syms );
Sym* SymGet( Syms* syms, InternId name );
void SymPutVar( Syms* syms, InternId name, ExprType type, u8 reg );
void SymPutFunc( Syms* syms, InternId name, FuncId funcid );
void SymFree( Syms* syms );

#endif