#ifndef LEXER_H
#define LEXER_H

#include "../common/src.h"

typedef struct Lexer {
	Src* src;
	u32 ln, col;
} Lexer;

void LexInit( Lexer* lexer, Src* src );
void LexReset( Lexer* lexer, x8* text );
void Lex( Lexer* lexer );
void LexFree( Lexer* lexer );

#endif