#ifndef PARSER_H
#define PARSER_H

typedef struct parser Parser;

Parser *newParser();

Parser *parser(Parser *parserE,char* line,char limitador);

void freeParser(Parser *parserE);

char** getTokens(Parser * parserE);

void printTokens(char **tokens);


int getNumTokens(Parser * parserE);

#endif 
