#ifndef HASH_H
#define HASH_H

#include "typedefs.h"
#include "cfg.h"

/* HashTag is a namespace for keys. */
/* All hashes encode one prior to the payload. */
/* This lets us use the same table without id: hello and string: "hello" */
/* colliding. They co-exist as separate entries. */
/* The tag is used in hashing and for equality. */

typedef enum HashTag {
	HASH_ID  = 1, /* SOH */
	HASH_STR = 2, /* STX */
} HashTag;

u32 HashStart( HashTag tag );
u32 HashU8( u32 hash, u8 byte );
u32 HashEnd( u32 hash );

#endif