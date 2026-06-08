#ifndef PARSER_H
#define PARSER_H

#include "../lexer/lexer.h"

typedef struct Parser {
	Logs* logs;
	Lexer* lexer;
} Parser;

void ParserInit( Parser* parser, Logs* logs, Lexer* lexer );
// void Parse( Parser* parser );

#endif