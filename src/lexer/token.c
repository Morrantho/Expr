#include "token.h"

u8* TkGetType( Tk* tk ){
	static u8* types[ ] = { X_TKS( X_TK_STRS ) };
	return types[ tk->type ];
}