#ifndef VM_POST_NUM_H
#define VM_POST_NUM_H

#include "../../vm_internal.h"

static inline void VmPostIncNum( Vm* vm, Inst* i ){
	f64 b = VmGetNum( vm, i->b );
	VmSetNum( vm, i->b, b + 1 );
	VmSetNum( vm, i->a, b );
}

static inline void VmPostDecNum( Vm* vm, Inst* i ){
	f64 b = VmGetNum( vm, i->b );
	VmSetNum( vm, i->b, b - 1 );
	VmSetNum( vm, i->a, b );
}

#endif