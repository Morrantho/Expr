#include "deno.h"

Deno DenoGet( ParsePos pos, TkType type ){
	static u8 denos[ PARSEPOS_COUNT ][ TK_COUNT ] = {
		X_TKS( X_DENO_PRES )
		X_TKS( X_DENO_INFS )
		X_TKS( X_DENO_POSTS )
	};
	return denos[ pos ][ type ];
}

u8* DenoGetName( Deno deno ){
	static u8* names[ ] = { X_DENOS( X_DENO_NAMES ) };
	return names[ deno ];
}