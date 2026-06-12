#include "opcode.h"

Op* OpGetUnary( ExprType rhs_type, TkType tk_type ){
	static Op unarys[ EXPR_COUNT ][ TK_COUNT ] = {
		X_OPS_UNA( X_OP_UNA_INIT )
	};
	return &unarys[ rhs_type ][ tk_type ];
}

Op* OpGetPost( ExprType lhs_type, TkType tk_type ){
	static Op posts[ EXPR_COUNT ][ TK_COUNT ] = {
		X_OPS_POST( X_OP_POST_INIT )
	};
	return &posts[ lhs_type ][ tk_type ];
}

Op* OpGetBinary( ExprType lhs_type, ExprType rhs_type, TkType tk_type ){
	static Op binarys[ EXPR_COUNT ][ EXPR_COUNT ][ TK_COUNT ] = {
		X_OPS_BIN( X_OP_BIN_INIT )
	};
	return &binarys[ lhs_type ][ rhs_type ][ tk_type ];
}

u8* OpGetName( OpCode opcode ){
	static u8* ops[ ] = { X_OPS( X_OP_STRS ) };
	return ops[ opcode ];
}

u8* OpGetUnaryName( TkType tk_type ){
	switch( tk_type ){
		default: return ( u8* )"unknown operator";
		X_OPS_UNA( X_OP_NAME_CASE )
	}
}

u8* OpGetPostName( TkType tk_type ){
	switch( tk_type ){
		default: return ( u8* )"unknown operator";
		X_OPS_POST( X_OP_NAME_CASE )
	}
}

u8* OpGetBinaryName( TkType tk_type ){
	switch( tk_type ){
		default: return ( u8* )"unknown operator";
		X_OPS_BIN( X_OP_NAME_CASE )
	}
}