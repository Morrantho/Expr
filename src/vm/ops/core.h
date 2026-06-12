#ifndef VM_CORE_H
#define VM_CORE_H

#include "../vm.h"

static inline void VmLoadConst( Vm* vm, Inst* i ){
	printf( "VmLoadConst: %p %p\n", ( void* )vm, ( void* )i );
}

#endif