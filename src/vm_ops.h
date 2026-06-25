#ifdef IMPL
/*CORE************************************************************************/
static inline void VmLoadConst( Vm* vm, Inst* i, Value* regs ){
	Const* c = ConstGet( vm->consts, InstGetBX( i ) );
	switch( c->type ){
		case CONST_NUM: VmNum( &regs[ i->a ], c->num ); return;
		case CONST_STR: VmStr( &regs[ i->a ], c->str ); return;
	}
	// Halt( ERR_LOADCONST );
}

static inline void VmJmp( Vm* vm, Inst* i, Value* regs ){ ( void )( regs );
	vm->ip = vm->base + InstGetBX( i );
}

static inline void VmJz( Vm* vm, Inst* i, Value* regs ){
	if( regs[ i->a ].num == 0 ) vm->ip = vm->base + InstGetBX( i );
}

static inline void VmJnz( Vm* vm, Inst* i, Value* regs ){
	if( regs[ i->a ].num != 0 ) vm->ip = vm->base + InstGetBX( i );
}

static inline void VmMov( Vm* vm, Inst* i, Value* regs ){ ( void )( vm );
	regs[ i->a ] = regs[ i->b ];
}
/*UNARY***********************************************************************/
static inline void VmNotNum( Value* a, Value* b ){
	VmNum( a, !b->num );
}

static inline void VmNegNum( Value* a, Value* b ){
	VmNum( a, -b->num );
}

static inline void VmBnotNum( Value* a, Value* b ){
	VmNum( a, ~( x64 )b->num );
}

static inline void VmPreIncNum( Value* a, Value* b ){
	++b->num;
	VmNum( a, b->num );
}

static inline void VmPreDecNum( Value* a, Value* b ){
	--b->num;
	VmNum( a, b->num );
}

static inline void VmRoundNum( Value* a, Value* b ){
	f64 n = b->num;
	x64 w = n;
	f64 f = n - w;
	VmNum( a, w + ( f >= 0.5 ) - ( f <= -0.5 ) );
}

static inline void VmCeilNum( Value* a, Value* b ){
	f64 n = b->num;
	x64 w = n;
	VmNum( a, w + ( w < n ) );
}

static inline void VmFloorNum( Value* a, Value* b ){
	f64 n = b->num;
	x64 w = n;
	VmNum( a, w - ( w > n ) );
}
/*POST************************************************************************/
static inline void VmPostIncNum( Value* a, Value* b ){
	f64 n = b->num;
	VmNum( a, n );
	++b->num;
}

static inline void VmPostDecNum( Value* a, Value* b ){
	f64 n = b->num;
	VmNum( a, n );
	--b->num;
}
/*BINARY NUM******************************************************************/
static inline void VmNotEqNum( Value* a, Value* b, Value* c ){
	VmNum( a, b->num != c->num );
}

static inline void VmModNum( Value* a, Value* b, Value* c ){
	VmNum( a, ( x64 )b->num % ( x64 )c->num );
}

static inline void VmBandNum( Value* a, Value* b, Value* c ){
	VmNum( a, ( x64 )b->num & ( x64 )c->num );
}

static inline void VmAndNum( Value* a, Value* b, Value* c ){
	VmNum( a, b->num && c->num );
}

static inline void VmMulNum( Value* a, Value* b, Value* c ){
	VmNum( a, b->num * c->num );
}

static inline void VmAddNum( Value* a, Value* b, Value* c ){
	VmNum( a, b->num + c->num );
}

static inline void VmSubNum( Value* a, Value* b, Value* c ){
	VmNum( a, b->num - c->num );
}

static inline void VmDivNum( Value* a, Value* b, Value* c ){
	VmNum( a, b->num / c->num );
}

static inline void VmLtNum( Value* a, Value* b, Value* c ){
	VmNum( a, b->num < c->num );
}

static inline void VmLshNum( Value* a, Value* b, Value* c ){
	VmNum( a, ( x64 )b->num << ( x64 )c->num );
}

static inline void VmLteNum( Value* a, Value* b, Value* c ){
	VmNum( a, b->num <= c->num );
}

static inline void VmCmpNum( Value* a, Value* b, Value* c ){
	VmNum( a, b->num == c->num );
}

static inline void VmGtNum( Value* a, Value* b, Value* c ){
	VmNum( a, b->num > c->num );
}

static inline void VmRshNum( Value* a, Value* b, Value* c ){
	VmNum( a, ( x64 )b->num >> ( x64 )c->num );
}

static inline void VmGteNum( Value* a, Value* b, Value* c ){
	VmNum( a, b->num >= c->num );
}

static inline void VmBxorNum( Value* a, Value* b, Value* c ){
	VmNum( a, ( x64 )b->num ^ ( x64 )c->num );
}

static inline void VmBorNum( Value* a, Value* b, Value* c ){
	VmNum( a, ( x64 )b->num | ( x64 )c->num );
}

static inline void VmOrNum( Value* a, Value* b, Value* c ){
	VmNum( a, b->num || c->num );
}

static inline void VmModEqNum( Value* a, Value* b, Value* c ){
	b->num = ( x64 )b->num % ( x64 )c->num;
	VmNum( a, b->num );
}

static inline void VmBandEqNum( Value* a, Value* b, Value* c ){
	b->num = ( x64 )b->num & ( x64 )c->num;
	VmNum( a, b->num );
}

static inline void VmMulEqNum( Value* a, Value* b, Value* c ){
	b->num *= c->num;
	VmNum( a, b->num );
}

static inline void VmAddEqNum( Value* a, Value* b, Value* c ){
	b->num += c->num;
	VmNum( a, b->num );
}

static inline void VmSubEqNum( Value* a, Value* b, Value* c ){
	b->num -= c->num;
	VmNum( a, b->num );
}

static inline void VmDivEqNum( Value* a, Value* b, Value* c ){
	b->num /= c->num;
	VmNum( a, b->num );
}

static inline void VmLshEqNum( Value* a, Value* b, Value* c ){
	b->num = ( x64 )b->num << ( x64 )c->num;
	VmNum( a, b->num );
}

static inline void VmRshEqNum( Value* a, Value* b, Value* c ){
	b->num = ( x64 )b->num >> ( x64 )c->num;
	VmNum( a, b->num );
}

static inline void VmBxorEqNum( Value* a, Value* b, Value* c ){
	b->num = ( x64 )b->num ^ ( x64 )c->num;
	VmNum( a, b->num );
}

static inline void VmBorEqNum( Value* a, Value* b, Value* c ){
	b->num = ( x64 )b->num | ( x64 )c->num;
	VmNum( a, b->num );
}

static inline void VmPowNum( Value* a, Value* b, Value* c ){
	x64 base = b->num;
	x64 exp = c->num;
	VmNum( a, VmPowX64( base, exp ) );
}
/*BINARY STR******************************************************************/
static inline void VmCmpStr( Value* a, Value* b, Value* c ){
	VmNum( a, b->str == c->str );
}
#endif