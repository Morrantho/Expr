#include "lexer.h"

void LexInit( Lexer* lexer, Src* src ){
	lexer->src = src;
	lexer->ln = lexer->col = 1;
}

void LexReset( Lexer* lexer, x8* text ){
	lexer->src->text = text;
	lexer->ln = lexer->col = 1;
}

void Lex( Lexer* lexer ){

}

void LexFree( Lexer* lexer ){

}