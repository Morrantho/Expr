#ifdef IMPL
/*CORE************************************************************************/
static void VmLoadConst( Vm* vm, Inst* i ){
	u16 idx = ( u16 )( i->b << 8 ) | i->c;
	Const* c = &vm->consts->data[ idx ];
	switch( c->type ){
		case CONST_NUM: VmSetNum( vm, i->a, c->num ); return;
		case CONST_STR: VmSetStr( vm, i->a, c->str ); return;
	}
}
/*UNARY***********************************************************************/
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
/*POST************************************************************************/
static inline void VmPostIncNum( Vm* vm, Inst* i ){
	f64 b = VmGetNum( vm, i->b );
	VmSetNum( vm, i->b, b + 1 );
	VmSetNum( vm, i->a, b );
}

static inline void VmPostDecNum( Vm* vm, Inst* i ){
	f64 b = VmGetNum( vm, i->b );
	VmSetNum( vm, i->b, b - 1 );
	VmSetNum( vm, i->a, b );
}
/*BINARY**********************************************************************/
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