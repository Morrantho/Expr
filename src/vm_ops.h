#ifdef IMPL
/*NATIVES*********************************************************************/
static inline void VmPrint( Vm* vm, Inst* i ){
	Value* args = &vm->regs[ i->a + 1 ];
	for( u8 arg = 0; arg < i->b; arg++ ){
		if( arg ) printf( " " );
		Value* value = &args[ arg ];
		switch( value->type ){
			case VALUE_NULL:
				printf( "NULL" ); break;
			case VALUE_NUM:
				printf( "%.15g", value->num ); break;
			case VALUE_STR:
				printf( "%s", InternGetRaw( vm->interns, value->str ) ); break;
			case VALUE_FN:			
				printf( "%p", ( void* )FnGet( vm->fns, value->fn ) ); break;
		}
	}
	printf( "\n" );
	vm->regs[ i->a ].type = VALUE_NULL;
}

static inline void VmDump( Vm* vm, Inst* i ){
	InstDump( vm->insts );
	vm->regs[ i->a ].type = VALUE_NULL;
}

static inline void VmType( Vm* vm, Inst* i ){ /* lazy intern for now */
	Value* arg = &vm->regs[ i->a + 1 ];
	u8* name = ValueGetName( arg->type );
	vm->regs[ i->a ].type = VALUE_STR;
	vm->regs[ i->a ].str = InternPut( vm->interns, name, HASH_STR );
}
/*CORE************************************************************************/
static inline void VmLoadConst( Vm* vm, Inst* i ){
	vm->regs[ i->a ] = *ConstGet( vm->consts, InstGetBX( i ) );
}

static inline void VmJmp( Vm* vm, Inst* i ){
	vm->ip = vm->insts->data + InstGetBX( i );
}

static inline void VmJz( Vm* vm, Inst* i ){
	if( vm->regs[ i->a ].num != 0 ) return;
	vm->ip = vm->insts->data + InstGetBX( i );
}

static inline void VmJnz( Vm* vm, Inst* i ){
	if( vm->regs[ i->a ].num == 0 ) return;
	vm->ip = vm->insts->data + InstGetBX( i );
}

static inline void VmMov( Vm* vm, Inst* i ){ ( void )( vm );
	vm->regs[ i->a ] = vm->regs[ i->b ];
}

static inline void VmReturn( Vm* vm, Inst* i ){
	Value ret = vm->regs[ i->a ];
	Frame* frame = VmFramePop( vm );
	vm->ip = frame->ip;
	vm->regs = frame->regs;
	vm->regs[ frame->ret ] = ret;
}

static inline void VmCallNative( Vm* vm, Inst* i, NativeIdx native ){
	switch( ( NativeType )native ){
		case NATIVE_COUNT: return;
		X_NATIVES( X_NATIVE_CASE )
	}
}

static inline void VmCall( Vm* vm, Inst* i ){
	Value* callee = &vm->regs[ i->a ];
	if( callee->type != VALUE_FN ){ Halt( ERR_BADCALL ); }
	FnIdx fn_idx = callee->fn;
	if( fn_idx >= vm->fns->fn_len ){ Halt( ERR_BADFN ); }
	Fn* fn = FnGet( vm->fns, fn_idx );
	if( fn->target == INST_NONE ){ Halt( ERR_BADFN ); }
	if( i->b != fn->nargs ){ Halt( ERR_BADARGS ); }
	if( fn->target & FN_NATIVE ){ VmCallNative( vm, i, fn->target & FN_TARGET ); return; }
	Frame* frame = VmFramePush( vm );
	frame->ip = vm->ip;
	frame->regs = vm->regs;
	frame->ret = i->a;
	vm->regs += i->a + 1;
	if( vm->regs + fn->nregs > vm->reg_stack + VM_REG_CAP ){ Halt( ERR_REGOVERFLOW ); }
	vm->ip = vm->insts->data + fn->target;
}
/*UNARY***********************************************************************/
static inline void VmNot( Value* a, Value* b ){
	VmNum( a, !b->num );
}

static inline void VmNeg( Value* a, Value* b ){
	VmNum( a, -b->num );
}

static inline void VmBnot( Value* a, Value* b ){
	VmNum( a, ~( x64 )b->num );
}

static inline void VmPreInc( Value* a, Value* b ){
	++b->num;
	VmNum( a, b->num );
}

static inline void VmPreDec( Value* a, Value* b ){
	--b->num;
	VmNum( a, b->num );
}

static inline void VmRound( Value* a, Value* b ){
	f64 n = b->num;
	x64 w = n;
	f64 f = n - w;
	VmNum( a, w + ( f >= 0.5 ) - ( f <= -0.5 ) );
}

static inline void VmCeil( Value* a, Value* b ){
	f64 n = b->num;
	x64 w = n;
	VmNum( a, w + ( w < n ) );
}

static inline void VmFloor( Value* a, Value* b ){
	f64 n = b->num;
	x64 w = n;
	VmNum( a, w - ( w > n ) );
}
/*POST************************************************************************/
static inline void VmPostInc( Value* a, Value* b ){
	f64 n = b->num;
	VmNum( a, n );
	++b->num;
}

static inline void VmPostDec( Value* a, Value* b ){
	f64 n = b->num;
	VmNum( a, n );
	--b->num;
}
/*BINARY**********************************************************************/
static inline void VmEq( Value* a, Value* b, Value* c );

static inline void VmNotEq( Value* a, Value* b, Value* c ){
	VmEq( a, b, c );
	a->num = !a->num;
}

static inline void VmMod( Value* a, Value* b, Value* c ){
	VmNum( a, ( x64 )b->num % ( x64 )c->num );
}

static inline void VmBand( Value* a, Value* b, Value* c ){
	VmNum( a, ( x64 )b->num & ( x64 )c->num );
}

static inline void VmAnd( Value* a, Value* b, Value* c ){
	VmNum( a, b->num && c->num );
}

static inline void VmMul( Value* a, Value* b, Value* c ){
	VmNum( a, b->num * c->num );
}

static inline void VmAdd( Value* a, Value* b, Value* c ){
	VmNum( a, b->num + c->num );
}

static inline void VmSub( Value* a, Value* b, Value* c ){
	VmNum( a, b->num - c->num );
}

static inline void VmDiv( Value* a, Value* b, Value* c ){
	VmNum( a, b->num / c->num );
}

static inline void VmLt( Value* a, Value* b, Value* c ){
	VmNum( a, b->num < c->num );
}

static inline void VmLsh( Value* a, Value* b, Value* c ){
	VmNum( a, ( x64 )b->num << ( x64 )c->num );
}

static inline void VmLte( Value* a, Value* b, Value* c ){
	VmNum( a, b->num <= c->num );
}

static inline void VmEq( Value* a, Value* b, Value* c ){
	a->type = VALUE_NUM;
	if( b->type != c->type ){ a->num = 0; return; }
	switch( b->type ){
		case VALUE_NULL: a->num = 1; return;
		case VALUE_NUM: a->num = b->num == c->num; return;
		case VALUE_STR: a->num = b->str == c->str; return;
		case VALUE_FN: a->num = b->fn == c->fn; return;
	}
}

static inline void VmGt( Value* a, Value* b, Value* c ){
	VmNum( a, b->num > c->num );
}

static inline void VmRsh( Value* a, Value* b, Value* c ){
	VmNum( a, ( x64 )b->num >> ( x64 )c->num );
}

static inline void VmGte( Value* a, Value* b, Value* c ){
	VmNum( a, b->num >= c->num );
}

static inline void VmBxor( Value* a, Value* b, Value* c ){
	VmNum( a, ( x64 )b->num ^ ( x64 )c->num );
}

static inline void VmBor( Value* a, Value* b, Value* c ){
	VmNum( a, ( x64 )b->num | ( x64 )c->num );
}

static inline void VmOr( Value* a, Value* b, Value* c ){
	VmNum( a, b->num || c->num );
}

static inline void VmModEq( Value* a, Value* b, Value* c ){
	b->num = ( x64 )b->num % ( x64 )c->num;
	VmNum( a, b->num );
}

static inline void VmBandEq( Value* a, Value* b, Value* c ){
	b->num = ( x64 )b->num & ( x64 )c->num;
	VmNum( a, b->num );
}

static inline void VmMulEq( Value* a, Value* b, Value* c ){
	b->num *= c->num;
	VmNum( a, b->num );
}

static inline void VmAddEq( Value* a, Value* b, Value* c ){
	b->num += c->num;
	VmNum( a, b->num );
}

static inline void VmSubEq( Value* a, Value* b, Value* c ){
	b->num -= c->num;
	VmNum( a, b->num );
}

static inline void VmDivEq( Value* a, Value* b, Value* c ){
	b->num /= c->num;
	VmNum( a, b->num );
}

static inline void VmLshEq( Value* a, Value* b, Value* c ){
	b->num = ( x64 )b->num << ( x64 )c->num;
	VmNum( a, b->num );
}

static inline void VmRshEq( Value* a, Value* b, Value* c ){
	b->num = ( x64 )b->num >> ( x64 )c->num;
	VmNum( a, b->num );
}

static inline void VmBxorEq( Value* a, Value* b, Value* c ){
	b->num = ( x64 )b->num ^ ( x64 )c->num;
	VmNum( a, b->num );
}

static inline void VmBorEq( Value* a, Value* b, Value* c ){
	b->num = ( x64 )b->num | ( x64 )c->num;
	VmNum( a, b->num );
}

static inline void VmPow( Value* a, Value* b, Value* c ){
	x64 base = b->num;
	x64 exp = c->num;
	VmNum( a, VmPowX64( base, exp ) );
}
#endif