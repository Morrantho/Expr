#ifndef INTERN_H
#define INTERN_H

#include <string.h>
#include "../typedefs.h"
#include "../aob/aob.h"
#include "../hash/hash.h"
typedef u32 InternId;
/* Only exists so internal calls arent 30 args long. */
typedef struct InternKey {
	HashTag tag;
	u8* src;
	u32 len;
	u32 hash;
} InternKey;
/* We could store the tag directly in the tables' AOB, or store it here. */
/* We have a bigger struct and waste bytes this way, but it might help */
/* with cache locality since we have to check its tag before a final memcmp */
/* If tags dont match, we don't memcmp, so no need to touch the AOB. */
typedef struct Intern {
	Offset offset;		/* table->aob->bytes + offset = raw */
	u32 hash;
	u32 len;			/* id / str len */
	u8 tag;				/* HashTag. HASH_STR or HASH_ID */
} Intern;

typedef struct Interns {
	u32* slots;			/* We probe this. indices = hash & mask. values = entry indices  */
	Intern* entries;	/* The entries for lookup comparisons */
	Aob aob;			/* raw ids / strings go here. */
	u32 slot_cap;		/* entry_cap << 1 */
	u32 slot_half_cap;  /* slot_cap >> 1 */
	u32 entry_cap;		/* slots_cap >> 1  */
	u32 len;			/* shared size */
} Interns;

void InternInit( Interns* interns );
InternId InternPutId( Interns* interns, u8* src, u32 len, u32 hash );
InternId InternPutStr( Interns* interns, u8* src, u32 len, u32 hash );
u8* InternGet( Interns* interns, InternId id );
void InternFree( Interns* interns );

#endif