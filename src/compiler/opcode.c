#include "opcode.h"

Op OpGetUnary( ExprType rhs_type, TkType tk_type ){
	static Op unarys[ EXPR_COUNT ][ TK_COUNT ] = {
		X_OPS_UNA( X_OP_UNA_INIT )
	};
	return unarys[ rhs_type ][ tk_type ];
}

Op OpGetPost( ExprType lhs_type, TkType tk_type ){
	static Op posts[ EXPR_COUNT ][ TK_COUNT ] = {
		X_OPS_POST( X_OP_POST_INIT )
	};
	return posts[ lhs_type ][ tk_type ];
}

Op OpGetBinary( ExprType lhs_type, ExprType rhs_type, TkType tk_type ){
	static Op binarys[ EXPR_COUNT ][ EXPR_COUNT ][ TK_COUNT ] = {
		X_OPS_BIN( X_OP_BIN_INIT )
	};
	return binarys[ lhs_type ][ rhs_type ][ tk_type ];
}