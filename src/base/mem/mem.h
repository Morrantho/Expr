#ifndef MEM_H
#define MEM_H

#include "../typedefs.h"
#include "../diag/err.h"

void* MemAlloc( u32 szof, u32 count );
void* MemCalloc( u32 szof, u32 count );
void* MemRealloc( void* block, u32 szof, u32 count );
void MemFree( void* block );

#endif