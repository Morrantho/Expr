#include "token.h"

u8* TkGetName( TkType type ){
	static u8* types[ ] = { X_TKS( X_TK_STRS ) };
	return types[ type ];
}