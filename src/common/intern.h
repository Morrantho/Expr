#ifndef INTERN_H
#define INTERN_H

#include <string.h>
#include "typedefs.h"
#include "aob.h"
#include "hash.h"
typedef u32 InternOffset;
/* We could store the tag directly in the tables' AOB, or store it here. */
/* We have a bigger struct and waste bytes this way, but it might help */
/* with cache locality since we have to check its tag before a final memcmp */
/* If tags dont match, we don't memcmp, so no need to touch the AOB. */
typedef struct Intern {
	Offset offset;		/* table->bytes->bytes + offset = raw */
	u32 hash;
	u32 len;			/* id / str len */
	u8 tag;				/* HashTag. HASH_STR or HASH_ID */
} Intern;

typedef struct Interns {
	Intern* entries;
	Aob aob;			/* raw ids / strings go here. */
	u32 len;
	u32 cap;
	u32 half_cap;		/* might as well cache it, we grow at 50% load */
} Interns;

void InternInit( Interns* table );
InternOffset InternPut( Interns* table, HashTag tag, u8* src, u32 len, u32 hash );
InternOffset InternPutId( Interns* table, u8* src, u32 len, u32 hash );
InternOffset InternPutStr( Interns* table, u8* src, u32 len, u32 hash );
u8* InternGetRaw( Interns* table, InternOffset off );
void InternFree( Interns* table );

#endif