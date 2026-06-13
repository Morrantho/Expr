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

static inline void VmSetStr( Vm* vm, u8 reg, InternId str ){
	Value* value = &vm->regs[ reg ];
	value->type = VALUE_STR;
	value->str = str;
}

static inline InternId VmGetStr( Vm* vm, u8 reg ){
	return vm->regs[ reg ].str;
}

#endif