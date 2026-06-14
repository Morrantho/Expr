#ifndef FNSYM_H
#define FNSYM_H

#include "../base/intern/intern.h"
#include "func.h"

typedef struct FnSym {
	InternId name;
	FuncId fn_id;
} FnSym;

typedef struct FnSyms {
	FnSym* data;
	u32 len;
	u32 cap;
} FnSyms;

void FnSymInit( FnSyms* syms );
void FnSymPut( FnSyms* syms, InternId name, FuncId fn_id );
FnSym* FnSymGet( FnSyms* syms, InternId name );
void FnSymFree( FnSyms* syms );

#endif