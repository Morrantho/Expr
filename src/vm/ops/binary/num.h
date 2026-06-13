#ifndef VM_BINARY_NUM_H
#define VM_BINARY_NUM_H

#include <math.h>
#include "../../vm_internal.h"

static inline void VmNotEqNum( Vm* vm, Inst* i ){
	VmSetNum( vm, i->a, VmGetNum( vm, i->b ) != VmGetNum( vm, i->c ) );
}

static inline void VmModNum( Vm* vm, Inst* i ){
	VmSetNum( vm, i->a, ( x64 )VmGetNum( vm, i->b ) % ( x64 )VmGetNum( vm, i->c ) );
}

static inline void VmBandNum( Vm* vm, Inst* i ){
	VmSetNum( vm, i->a, ( x64 )VmGetNum( vm, i->b ) & ( x64 )VmGetNum( vm, i->c ) );
}

static inline void VmAndNum( Vm* vm, Inst* i ){
	VmSetNum( vm, i->a, VmGetNum( vm, i->b ) && VmGetNum( vm, i->c ) );
}

static inline void VmMulNum( Vm* vm, Inst* i ){
	VmSetNum( vm, i->a, VmGetNum( vm, i->b ) * VmGetNum( vm, i->c ) );
}

static inline void VmAddNum( Vm* vm, Inst* i ){
	VmSetNum( vm, i->a, VmGetNum( vm, i->b ) + VmGetNum( vm, i->c ) );
}

static inline void VmSubNum( Vm* vm, Inst* i ){
	VmSetNum( vm, i->a, VmGetNum( vm, i->b ) - VmGetNum( vm, i->c ) );
}

static inline void VmDivNum( Vm* vm, Inst* i ){
	VmSetNum( vm, i->a, VmGetNum( vm, i->b ) / VmGetNum( vm, i->c ) );
}

static inline void VmLtNum( Vm* vm, Inst* i ){
	VmSetNum( vm, i->a, VmGetNum( vm, i->b ) < VmGetNum( vm, i->c ) );
}

static inline void VmLshNum( Vm* vm, Inst* i ){
	VmSetNum( vm, i->a, ( x64 )VmGetNum( vm, i->b ) << ( x64 )VmGetNum( vm, i->c ) );
}

static inline void VmLteNum( Vm* vm, Inst* i ){
	VmSetNum( vm, i->a, VmGetNum( vm, i->b ) <= VmGetNum( vm, i->c ) );
}

static inline void VmIsEqNum( Vm* vm, Inst* i ){
	VmSetNum( vm, i->a, VmGetNum( vm, i->b ) == VmGetNum( vm, i->c ) );
}

static inline void VmGtNum( Vm* vm, Inst* i ){
	VmSetNum( vm, i->a, VmGetNum( vm, i->b ) > VmGetNum( vm, i->c ) );
}

static inline void VmRshNum( Vm* vm, Inst* i ){
	VmSetNum( vm, i->a, ( x64 )VmGetNum( vm, i->b ) >> ( x64 )VmGetNum( vm, i->c ) );
}

static inline void VmGteNum( Vm* vm, Inst* i ){
	VmSetNum( vm, i->a, VmGetNum( vm, i->b ) >= VmGetNum( vm, i->c ) );
}

static inline void VmBxorNum( Vm* vm, Inst* i ){
	VmSetNum( vm, i->a, ( x64 )VmGetNum( vm, i->b ) ^ ( x64 )VmGetNum( vm, i->c ) );
}

static inline void VmBorNum( Vm* vm, Inst* i ){
	VmSetNum( vm, i->a, ( x64 )VmGetNum( vm, i->b ) | ( x64 )VmGetNum( vm, i->c ) );
}

static inline void VmOrNum( Vm* vm, Inst* i ){
	VmSetNum( vm, i->a, VmGetNum( vm, i->b ) || VmGetNum( vm, i->c ) );
}

static inline void VmModEqNum( Vm* vm, Inst* i ){
	f64 b = ( x64 )VmGetNum( vm, i->b ) % ( x64 )VmGetNum( vm, i->c );
	VmSetNum( vm, i->b, b );
	VmSetNum( vm, i->a, b );
}

static inline void VmBandEqNum( Vm* vm, Inst* i ){
	f64 b = ( x64 )VmGetNum( vm, i->b ) & ( x64 )VmGetNum( vm, i->c );
	VmSetNum( vm, i->b, b );
	VmSetNum( vm, i->a, b );
}

static inline void VmMulEqNum( Vm* vm, Inst* i ){
	f64 b = VmGetNum( vm, i->b ) * VmGetNum( vm, i->c );
	VmSetNum( vm, i->b, b );
	VmSetNum( vm, i->a, b );
}

static inline void VmAddEqNum( Vm* vm, Inst* i ){
	f64 b = VmGetNum( vm, i->b ) + VmGetNum( vm, i->c );
	VmSetNum( vm, i->b, b );
	VmSetNum( vm, i->a, b );
}

static inline void VmSubEqNum( Vm* vm, Inst* i ){
	f64 b = VmGetNum( vm, i->b ) - VmGetNum( vm, i->c );
	VmSetNum( vm, i->b, b );
	VmSetNum( vm, i->a, b );
}

static inline void VmDivEqNum( Vm* vm, Inst* i ){
	f64 b = VmGetNum( vm, i->b ) / VmGetNum( vm, i->c );
	VmSetNum( vm, i->b, b );
	VmSetNum( vm, i->a, b );
}

static inline void VmLshEqNum( Vm* vm, Inst* i ){
	f64 b = ( x64 )VmGetNum( vm, i->b ) << ( x64 )VmGetNum( vm, i->c );
	VmSetNum( vm, i->b, b );
	VmSetNum( vm, i->a, b );
}

static inline void VmRshEqNum( Vm* vm, Inst* i ){
	f64 b = ( x64 )VmGetNum( vm, i->b ) >> ( x64 )VmGetNum( vm, i->c );
	VmSetNum( vm, i->b, b );
	VmSetNum( vm, i->a, b );
}

static inline void VmBxorEqNum( Vm* vm, Inst* i ){
	f64 b = ( x64 )VmGetNum( vm, i->b ) ^ ( x64 )VmGetNum( vm, i->c );
	VmSetNum( vm, i->b, b );
	VmSetNum( vm, i->a, b );
}

static inline void VmBorEqNum( Vm* vm, Inst* i ){
	f64 b = ( x64 )VmGetNum( vm, i->b ) | ( x64 )VmGetNum( vm, i->c );
	VmSetNum( vm, i->b, b );
	VmSetNum( vm, i->a, b );
}

static inline void VmPowNum( Vm* vm, Inst* i ){
	x64 base = VmGetX64( vm, i->b );
	x64 exp = VmGetX64( vm, i->c );
	VmSetX64( vm, i->a, VmPowX64( base, exp ) );
}

#endif