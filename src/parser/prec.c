#include "prec.h"

Prec PrecGet( TkType type ){
	static u8 precs[ ] = { X_TKS( X_TK_PRECS ) };
	return precs[ type ];
}