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
	u8 expr_type;
	u8 reg;
} Sym;

typedef struct Syms {
	Sym* data;
	u32 len;
	u32 cap;
} Syms;

void SymInit( Syms* syms );
Sym* SymGet( Syms* syms, InternId name );
void SymPut( Syms* syms, InternId name, ExprType type, u8 reg );
void SymFree( Syms* syms );

#endif