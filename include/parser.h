#ifndef PARSER_H
#define PARSER_H

typedef struct parser Parser;

Parser *newParser();

Parser *parser(Parser *parserE,char* line);

void freeParser(Parser *parserE);

char** getTokens(Parser * parserE);

int getNumTokens(Parser * parserE);

#endif 
