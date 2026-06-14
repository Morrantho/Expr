#ifndef COMPILER_EXCLUDE_H
#define COMPILER_EXCLUDE_H

#define X_OPS_EXCLUDE( X )\
	X( ERR,  _, _, _, _, _, "error" )\
	X( HALT, _, _, _, _, _, "halt" )\
	X( RET,  _, _, _, _, _, "return" )

#endif