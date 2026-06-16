#ifndef COMPILER_CORE_H
#define COMPILER_CORE_H

#define X_OPS_CORE( X )\
	X( LOADC, LoadConst, _, _, _, _, "load constant" )\
	X( RET,   Return,    _, _, _, _, "return" )\
	X( CALL,  Call,      _, _, _, _, "call" )\
	X( ARG,   Arg,       _, _, _, _, "arg" )
	
#endif