#include "deno.h"

Deno DenoGet( Pos pos, TkType type ){
	static u8 denos[ POS_COUNT ][ TK_COUNT ] = {
		X_TKS( X_DENO_PRES )
		X_TKS( X_DENO_INFS )
		X_TKS( X_DENO_POSTS )
	};
	return denos[ pos ][ type ];
}