#include "token.h"

x8* TkGetType( Tk* tk ){
	static x8* types[ ] = { X_TKS( X_TK_STRS ) };
	return types[ tk->type ];
}