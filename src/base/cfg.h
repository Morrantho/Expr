#ifndef CFG_H
#define CFG_H

#define U32( N )		( ( u32 )( N ) )
#define COUNT( N )		U32( N )
#define KB( N )			( U32( 1024 ) * U32( N ) )
#define PAGE( N )		( U32( 4096 ) * U32( N ) )

#define SRC_REPL_CAP	KB( 1 )
#define SRC_AOB_CAP		PAGE( 4 )
#define SRC_CAP			COUNT( 16 )

#define LOG_BUF_CAP		KB( 1 )
#define LOG_AOB_CAP		PAGE( 1 )
#define LOG_CAP			COUNT( 128 )

#define HASH_OFFSET		U32( 2166136261 )	/* FNV1a */
#define HASH_PRIME		U32( 16777619 )		/* FNV1a */

#define INTERN_AOB_CAP	PAGE( 4 )
#define INTERN_CAP		COUNT( 256 )

#define CONST_CAP		COUNT( 256 )

#define CMP_REG_CAP		COUNT( 256 )
#define CMP_REG_ERR		CMP_REG_CAP
#define INST_CAP		COUNT( 1024 )

#endif