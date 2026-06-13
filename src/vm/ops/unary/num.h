#ifndef VM_UNARY_NUM_H
#define VM_UNARY_NUM_H

#include "../../vm_internal.h"

static inline void VmNotNum( Vm* vm, Inst* i ){
	VmSetNum( vm, i->a, !VmGetNum( vm, i->b ) );
}

static inline void VmNegNum( Vm* vm, Inst* i ){
	VmSetNum( vm, i->a, -VmGetNum( vm, i->b ) );
}

static inline void VmBnotNum( Vm* vm, Inst* i ){
	VmSetNum( vm, i->a, ~( x64 )VmGetNum( vm, i->b ) );
}

static inline void VmPreIncNum( Vm* vm, Inst* i ){
	f64 b = VmGetNum( vm, i->b ) + 1;
	VmSetNum( vm, i->b, b );
	VmSetNum( vm, i->a, b );
}

static inline void VmPreDecNum( Vm* vm, Inst* i ){
	f64 b = VmGetNum( vm, i->b ) - 1;
	VmSetNum( vm, i->b, b );
	VmSetNum( vm, i->a, b );
}

static inline void VmRoundNum( Vm* vm, Inst* i ){
	f64 b = VmGetNum( vm, i->b );
	x64 w = b;
	f64 f = b - w;
	VmSetNum( vm, i->a, w + ( f >= 0.5 ) - ( f <= -0.5 ) );
}

static inline void VmCeilNum( Vm* vm, Inst* i ){
	f64 b = VmGetNum( vm, i->b );
	x64 w = b;
	VmSetNum( vm, i->a, w + ( w < b ) );
}

static inline void VmFloorNum( Vm* vm, Inst* i ){
	f64 b = VmGetNum( vm, i->b );
	x64 w = b;
	VmSetNum( vm, i->a, w - ( w > b ) );
}

#endif