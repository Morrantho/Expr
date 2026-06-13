#ifndef VM_CORE_H
#define VM_CORE_H

#include "../vm_internal.h"

static void VmLoadConst( Vm* vm, Inst* i ){
	u16 id = ( u16 )( i->b << 8 ) | i->c;
	Const* c = &vm->consts->items[ id ];
	switch( c->type ){
		case CONST_NUM: VmSetNum( vm, i->a, c->num ); return;
		case CONST_STR: VmSetStr( vm, i->a, c->str ); return;
	}
}

#endif