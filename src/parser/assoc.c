#include "assoc.h"

Assoc AssocGet( TkType type ){
	static u8 assocs[ ] = { X_TKS( X_TK_ASSOCS ) };
	return assocs[ type ];
}