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

static void VmCall( Vm* vm, Inst* i ){
	FuncId fn_id = ( u16 )( i->b << 8 ) | i->c;
	Func* fn = &vm->funcs->data[ fn_id ];
	Call* call = &vm->calls[ vm->call++ ];
	call->ip = vm->ip;
	call->frame = vm->frame;
	call->ret = i->a;
	Frame* last = VmGetFrame( vm );
	u8 start = last->end;
	u8 end = start + fn->nregs;
	VmPushFrame( vm, start, end );
	vm->ip = &vm->insts->code[ fn->start ];
}

static void VmArg( Vm* vm, Inst* i ){
	Frame* frame = VmGetFrame( vm );
	Value* src = VmGetValue( vm, i->b );
	Value* dst = &vm->regs[ frame->end + i->a ];
	*dst = *src;
}

static void VmReturn( Vm* vm, Inst* i ){
	Value ret = *VmGetValue( vm, i->a );
	Call* call = &vm->calls[ --vm->call ];
	vm->frame = call->frame;
	*VmGetValue( vm, call->ret ) = ret;
	vm->ip = call->ip;
}

#endif