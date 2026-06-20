#ifdef BASE
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
typedef char x8; typedef short x16; typedef int x32; typedef long long x64;
typedef uint8_t u8; typedef uint16_t u16; typedef uint32_t u32; typedef uint64_t u64;
typedef int8_t i8; typedef int16_t i16; typedef int32_t i32; typedef int64_t i64;
typedef float f32; typedef double f64;
typedef u32 Idx, Offset;
#endif
#include "cfg.h"
#include "err.h"
#include "mem.h"
#include "aob.h"
#include "src.h"
#include "log.h"
#include "intern.h"
#include "ascii.h"
#include "token.h"
#include "lexer.h"
#include "parsing.h"
#include "const.h"
// #include "fn.h"
#include "sym.h"
#include "op.h"
#include "inst.h"
#include "compiler.h"
#include "vm.h"