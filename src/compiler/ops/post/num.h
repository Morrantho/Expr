#ifndef COMPILER_POST_NUM_H
#define COMPILER_POST_NUM_H

#define X_OPS_POST_NUM_MUT_C( X )\
	X( POSTINC_NUM, PostIncNum, INC, NUM, _, NUM, "post-increment" ) /* a++ */\
	X( POSTDEC_NUM, PostDecNum, DEC, NUM, _, NUM, "post-decrement" ) /* a-- */

#define X_OPS_POST( X )\
	X_OPS_POST_NUM_MUT_C( X )

#endif