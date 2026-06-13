#ifndef VM_INTERNAL_H
#define VM_INTERNAL_H

#include "vm.h"

static inline void VmPushFrame( Vm* vm, u8 start, u8 end ){
	if( vm->frame == CMP_REG_CAP-1 ){ Throw( ERR_FRAMEOVERFLOW ); return; }
	if( end < start ){ Throw( ERR_BADFRAME ); return; }
	Frame* frame = &vm->frames[ ++vm->frame ];
	frame->start = start;
	frame->end = end;
}

static inline Frame* VmPopFrame( Vm* vm ){
	if( !vm->frame ) return &vm->frames[ 0 ];
	return &vm->frames[ vm->frame-- ];
}

static inline Frame* VmGetFrame( Vm* vm ){
	return &vm->frames[ vm->frame ];
}

static inline Value* VmGetValue( Vm* vm, u8 reg ){
	Frame* frame = VmGetFrame( vm );
	return &vm->regs[ ( u16 )frame->start + reg ];
}

static inline void VmSetNum( Vm* vm, u8 reg, f64 num ){
	Value* value = VmGetValue( vm, reg );
	value->type = VALUE_NUM;
	value->num = num;
}

static inline f64 VmGetNum( Vm* vm, u8 reg ){
	return VmGetValue( vm, reg )->num;
}

static inline void VmSetX64( Vm* vm, u8 reg, x64 num ){
	VmSetNum( vm, reg, num );
}

static inline x64 VmGetX64( Vm* vm, u8 reg ){
	return ( x64 )VmGetNum( vm, reg );
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
	Value* value = VmGetValue( vm, reg );
	value->type = VALUE_STR;
	value->str = str;
}

static inline InternId VmGetStr( Vm* vm, u8 reg ){
	return VmGetValue( vm, reg )->str;
}

#endif