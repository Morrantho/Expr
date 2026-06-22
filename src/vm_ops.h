#ifdef IMPL
/*CORE************************************************************************/
static void VmLoadConst( Vm* vm, Inst* i, Value* regs ){
	u16 idx = ( u16 )( i->b << 8 ) | i->c;
	Const* c = &vm->consts->data[ idx ];
	switch( c->type ){
		case CONST_NUM: VmNum( &regs[ i->a ], c->num ); return;
		case CONST_STR: VmStr( &regs[ i->a ], c->str ); return;
	}
	// Halt( ERR_LOADCONST );
}

static void VmMov( Vm* vm, Inst* i, Value* regs ){
	regs[ i->a ] = regs[ i->b ];
}

static void VmJmp( Vm* vm, Inst* i, Value* regs ){
	vm->ip = VmInstAt( vm, InstGetBX( i ) );
}

static void VmJz( Vm* vm, Inst* i, Value* regs ){
	if( regs[ i->a ].num == 0 ) vm->ip = VmInstAt( vm, InstGetBX( i ) );
}

static void VmJnz( Vm* vm, Inst* i, Value* regs ){
	if( regs[ i->a ].num != 0 ) vm->ip = VmInstAt( vm, InstGetBX( i ) );
}
/*UNARY***********************************************************************/
static inline void VmNotNum( Vm* vm, Inst* i, Value* regs ){
	VmNum( &regs[ i->a ], !regs[ i->b ].num );
}

static inline void VmNegNum( Vm* vm, Inst* i, Value* regs ){
	VmNum( &regs[ i->a ], -regs[ i->b ].num );
}

static inline void VmBnotNum( Vm* vm, Inst* i, Value* regs ){
	VmNum( &regs[ i->a ], ~( x64 )regs[ i->b ].num );
}

static inline void VmPreIncNum( Vm* vm, Inst* i, Value* regs ){
	Value* src = &regs[ i->b ];
	++src->num;
	VmNum( &regs[ i->a ], src->num );
}

static inline void VmPreDecNum( Vm* vm, Inst* i, Value* regs ){
	Value *src = &regs[ i->b ];
	--src->num;
	VmNum( &regs[ i->a ], src->num );
}

static inline void VmRoundNum( Vm* vm, Inst* i, Value* regs ){
	f64 b = regs[ i->b ].num;
	x64 w = b;
	f64 f = b - w;
	VmNum( &regs[ i->a ], w + ( f >= 0.5 ) - ( f <= -0.5 ) );
}

static inline void VmCeilNum( Vm* vm, Inst* i, Value* regs ){
	f64 b = regs[ i->b ].num;
	x64 w = b;
	VmNum( &regs[ i->a ], w + ( w < b ) );
}

static inline void VmFloorNum( Vm* vm, Inst* i, Value* regs ){
	f64 b = regs[ i->b ].num;
	x64 w = b;
	VmNum( &regs[ i->a ], w - ( w > b ) );
}
/*POST************************************************************************/
static inline void VmPostIncNum( Vm* vm, Inst* i, Value* regs ){
	Value* src = &regs[ i->b ];
	f64 b = src->num;
	VmNum( &regs[ i->a ], b );
	++src->num;
}

static inline void VmPostDecNum( Vm* vm, Inst* i, Value* regs ){
	Value* src = &regs[ i->b ];
	f64 b = src->num;
	VmNum( &regs[ i->a ], b );
	--src->num;
}
/*BINARY NUM******************************************************************/
static inline void VmNotEqNum( Vm* vm, Inst* i, Value* regs ){
	VmNum( &regs[ i->a ], regs[ i->b ].num != regs[ i->c ].num );
}

static inline void VmModNum( Vm* vm, Inst* i, Value* regs ){
	VmNum( &regs[ i->a ], ( x64 )regs[ i->b ].num % ( x64 )regs[ i->c ].num );
}

static inline void VmBandNum( Vm* vm, Inst* i, Value* regs ){
	VmNum( &regs[ i->a ], ( x64 )regs[ i->b ].num & ( x64 )regs[ i->c ].num );
}

static inline void VmAndNum( Vm* vm, Inst* i, Value* regs ){
	VmNum( &regs[ i->a ], regs[ i->b ].num && regs[ i->c ].num );
}

static inline void VmMulNum( Vm* vm, Inst* i, Value* regs ){
	VmNum( &regs[ i->a ], regs[ i->b ].num * regs[ i->c ].num );
}

static inline void VmAddNum( Vm* vm, Inst* i, Value* regs ){
	VmNum( &regs[ i->a ], regs[ i->b ].num + regs[ i->c ].num );
}

static inline void VmSubNum( Vm* vm, Inst* i, Value* regs ){
	VmNum( &regs[ i->a ], regs[ i->b ].num - regs[ i->c ].num );
}

static inline void VmDivNum( Vm* vm, Inst* i, Value* regs ){
	VmNum( &regs[ i->a ], regs[ i->b ].num / regs[ i->c ].num );
}

static inline void VmLtNum( Vm* vm, Inst* i, Value* regs ){
	VmNum( &regs[ i->a ], regs[ i->b ].num < regs[ i->c ].num );
}

static inline void VmLshNum( Vm* vm, Inst* i, Value* regs ){
	VmNum( &regs[ i->a ], ( x64 )regs[ i->b ].num << ( x64 )regs[ i->c ].num );
}

static inline void VmLteNum( Vm* vm, Inst* i, Value* regs ){
	VmNum( &regs[ i->a ], regs[ i->b ].num <= regs[ i->c ].num );
}

static inline void VmCmpNum( Vm* vm, Inst* i, Value* regs ){
	VmNum( &regs[ i->a ], regs[ i->b ].num == regs[ i->c ].num );
}

static inline void VmGtNum( Vm* vm, Inst* i, Value* regs ){
	VmNum( &regs[ i->a ], regs[ i->b ].num > regs[ i->c ].num );
}

static inline void VmRshNum( Vm* vm, Inst* i, Value* regs ){
	VmNum( &regs[ i->a ], ( x64 )regs[ i->b ].num >> ( x64 )regs[ i->c ].num );
}

static inline void VmGteNum( Vm* vm, Inst* i, Value* regs ){
	VmNum( &regs[ i->a ], regs[ i->b ].num >= regs[ i->c ].num );
}

static inline void VmBxorNum( Vm* vm, Inst* i, Value* regs ){
	VmNum( &regs[ i->a ], ( x64 )regs[ i->b ].num ^ ( x64 )regs[ i->c ].num );
}

static inline void VmBorNum( Vm* vm, Inst* i, Value* regs ){
	VmNum( &regs[ i->a ], ( x64 )regs[ i->b ].num | ( x64 )regs[ i->c ].num );
}

static inline void VmOrNum( Vm* vm, Inst* i, Value* regs ){
	VmNum( &regs[ i->a ], regs[ i->b ].num || regs[ i->c ].num );
}

static inline void VmModEqNum( Vm* vm, Inst* i, Value* regs ){
	Value* src = &regs[ i->b ];
	src->num = ( x64 )src->num % ( x64 )regs[ i->c ].num;
	VmNum( &regs[ i->a ], src->num );
}

static inline void VmBandEqNum( Vm* vm, Inst* i, Value* regs ){
	Value* src = &regs[ i->b ];
	src->num = ( x64 )src->num & ( x64 )regs[ i->c ].num;
	VmNum( &regs[ i->a ], src->num );
}

static inline void VmMulEqNum( Vm* vm, Inst* i, Value* regs ){
	Value* src = &regs[ i->b ];
	src->num *= regs[ i->c ].num;
	VmNum( &regs[ i->a ], src->num );
}

static inline void VmAddEqNum( Vm* vm, Inst* i, Value* regs ){
	Value* src = &regs[ i->b ];
	src->num += regs[ i->c ].num;
	VmNum( &regs[ i->a ], src->num );
}

static inline void VmSubEqNum( Vm* vm, Inst* i, Value* regs ){
	Value* src = &regs[ i->b ];
	src->num -= regs[ i->c ].num;
	VmNum( &regs[ i->a ], src->num );
}

static inline void VmDivEqNum( Vm* vm, Inst* i, Value* regs ){
	Value* src = &regs[ i->b ];
	src->num /= regs[ i->c ].num;
	VmNum( &regs[ i->a ], src->num );
}

static inline void VmLshEqNum( Vm* vm, Inst* i, Value* regs ){
	Value* src = &regs[ i->b ];
	src->num = ( x64 )src->num << ( x64 )regs[ i->c ].num;
	VmNum( &regs[ i->a ], src->num );
}

static inline void VmRshEqNum( Vm* vm, Inst* i, Value* regs ){
	Value* src = &regs[ i->b ];
	src->num = ( x64 )src->num >> ( x64 )regs[ i->c ].num;
	VmNum( &regs[ i->a ], src->num );
}

static inline void VmBxorEqNum( Vm* vm, Inst* i, Value* regs ){
	Value* src = &regs[ i->b ];
	src->num = ( x64 )src->num ^ ( x64 )regs[ i->c ].num;
	VmNum( &regs[ i->a ], src->num );
}

static inline void VmBorEqNum( Vm* vm, Inst* i, Value* regs ){
	Value* src = &regs[ i->b ];
	src->num = ( x64 )src->num | ( x64 )regs[ i->c ].num;
	VmNum( &regs[ i->a ], src->num );
}

static inline void VmPowNum( Vm* vm, Inst* i, Value* regs ){
	x64 base = regs[ i->b ].num;
	x64 exp = regs[ i->c ].num;
	VmNum( &regs[ i->a ], VmPowX64( base, exp ) );
}
/*BINARY STR******************************************************************/
static inline void VmCmpStr( Vm* vm, Inst* i, Value* regs ){
	VmNum( &regs[ i->a ], regs[ i->b ].str == regs[ i->c ].str );
}
#endif