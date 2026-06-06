#ifndef SRC_H
#define SRC_H

#include <string.h>
#include "typedefs.h"
#include "cfg.h"
#include "mem.h"
#include "aob.h"

typedef u32 SrcId;

typedef struct Src {
	Offset path;	/* filename */
	Offset text;	/* source code */
	u32 len;		/* source code length */
} Src;

typedef struct SrcList {
	Aob bytes;		/* raw file paths/names and source code go here. */
	Src* sources;	/* vector. we store offsets for lookups. */
	u32 len;		/* number of source files. useful when we get to include()s. */
	u32 cap;		/* current source file capacity before growing */
} SrcList;

void SrcInit( SrcList* list );
Src* SrcGet( SrcList* list, SrcId id );
u8* SrcGetPath( SrcList* list, SrcId src_id );
u8* SrcGetText( SrcList* list, SrcId src_id );
SrcId SrcLoad( SrcList* list, u8* path );
void SrcFree( SrcList* list );

#endif