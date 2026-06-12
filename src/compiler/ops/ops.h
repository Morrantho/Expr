#ifndef COMPILER_OPS_H
#define COMPILER_OPS_H

#include "exclude.h"
#include "core.h"
#include "unary/unary.h"
#include "post/post.h"
#include "binary/binary.h"

#define X_OPS( X )\
	X_OPS_EXCLUDE( X )\
	X_OPS_CORE( X )\
	X_OPS_UNA( X )\
	X_OPS_POST( X )\
	X_OPS_BIN( X )

#endif