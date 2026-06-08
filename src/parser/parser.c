#include "parser.h"

void ParserInit( Parser* parser, Logs* logs, Lexer* lexer ){
	parser->logs = logs;
	parser->lexer = lexer;
}

// void Parse( Parser* parser ){

// }