#ifndef SRC_H
#define SRC_H

#include <string.h>
#include "../common/typedefs.h"
#include "../common/cfg.h"
#include "../common/mem.h"

typedef u32 SrcId;

typedef struct Src {
	x8* path;
	x8* text;
	u32 len;
} Src;

typedef struct SrcList {
	Src* data;
	u32 len;
	u32 cap;
} SrcList;

void SrcInit( SrcList* list, u32 cap );
Src* SrcGet( SrcList* list, SrcId id );
Src* SrcLoad( SrcList* list, x8* path );
void SrcFree( SrcList* list );

#endif