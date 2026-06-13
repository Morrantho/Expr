#ifndef VM_INTERNAL_H
#define VM_INTERNAL_H

#include "vm.h"

static inline void VmSetNum( Vm* vm, u8 reg, f64 num ){
	Value* value = &vm->regs[ reg ];
	value->type = VALUE_NUM;
	value->num = num;
}

static inline f64 VmGetNum( Vm* vm, u8 reg ){
	return vm->regs[ reg ].num;
}

static inline void VmSetX64( Vm* vm, u8 reg, f64 num ){
	VmSetNum( vm, reg, ( x64 )num );
}

static inline x64 VmGetX64( Vm* vm, u8 reg ){
	return ( x64 )vm->regs[ reg ].num;
}

static inline x64 VmPowX64( x64 base, x64 exp ){
	if( exp < 0 ) return 0;
	x64 res = 1;
	for( ; exp; exp >>= 1 ){
		if( exp & 1 ) res *= base;
		base *= base;
	}
	return res;
}

static inline void VmSetStr( Vm* vm, u8 reg, InternId str ){
	Value* value = &vm->regs[ reg ];
	value->type = VALUE_STR;
	value->str = str;
}

static inline InternId VmGetStr( Vm* vm, u8 reg ){
	return vm->regs[ reg ].str;
}

#endif