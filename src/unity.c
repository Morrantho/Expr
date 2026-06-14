#include "base/diag/err.c"
#include "base/mem/mem.c"
#include "base/aob/aob.c"
#include "base/src/src.c"
#include "base/diag/log.c"
#include "base/hash/hash.c"

#include "base/intern/intern.c"

#include "lexer/token.c"
#include "lexer/lexer.c"

#include "parsing/prec.c"
#include "parsing/assoc.c"
#include "parsing/deno.c"

#include "base/const/const.c"

#include "compiler/func.c"
#include "compiler/expr.c"
#include "compiler/sym.c"
#include "compiler/opcode.c"
#include "compiler/inst.c"
#include "compiler/compiler.c"

#include "vm/vm.c"

#include "app/app.c"