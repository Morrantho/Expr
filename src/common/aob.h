#ifndef AOB_H
#define AOB_H

#include "typedefs.h"
#include "cfg.h"
#include "mem.h"

typedef u32 Offset;

typedef struct Aob {
	x8* bytes;
	u32 len;
	u32 cap;
} Aob;

void AobInit( Aob* aob, u32 cap );
void AobReset( Aob* aob );
Offset AobPush( Aob* aob, u32 len );
x8* AobGet( Aob* aob, Offset offset );
void AobFree( Aob* aob );

#endif