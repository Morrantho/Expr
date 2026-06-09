#ifndef SRC_H
#define SRC_H

#include <string.h>
#include "../aob/aob.h"

typedef u32 SrcId;

typedef struct Src {
	Offset path;	/* filename */
	Offset text;	/* source code */
	u32 len;		/* source code length */
} Src;

typedef struct Srcs {
	Aob aob;		/* raw file paths/names and source code go here. */
	Src* sources;	/* vector. we store offsets for lookups. */
	u32 len;		/* number of source files. useful when we get to include()s. */
	u32 cap;		/* current source file capacity before growing */
} Srcs;

void SrcInit( Srcs* list );
Src* SrcGet( Srcs* list, SrcId id );
u8* SrcGetPath( Srcs* list, SrcId src_id );
u8* SrcGetText( Srcs* list, SrcId src_id );
SrcId SrcLoad( Srcs* list, u8* path );
void SrcFree( Srcs* list );

#endif