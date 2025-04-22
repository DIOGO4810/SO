#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "parser.h"

#define MaxTokensSize 12

struct parser
{
    int fd;

    char* line;

    char **tokens;

    int numtokens;
};


// Função que cria e aloca memoria para a estrutura e os campos do Parser
Parser *newParser()
{
    Parser *parserE = malloc(sizeof(struct parser));
    parserE->tokens = malloc(MaxTokensSize * sizeof(char *));    
    parserE->line = NULL;
    parserE->numtokens = 0;
    return parserE;
}

Parser *parser(Parser *parserE,char* line,char limitador)
{


    parserE->line = line;

    char *lineCopy = line;

    int i = 0;

    // Divide a linha em tokens usando strsep
    char *token = strsep(&lineCopy, &limitador);
    while (token != NULL && i < MaxTokensSize)
    {
        // Armazenar o token no array
        parserE->tokens[i++] = token;
        token = strsep(&lineCopy, &limitador);
    }
    parserE->numtokens = i-1;
    return parserE;
}

// Função que dá free da memoria para a estrutura e os campos do Parser
void freeParser(Parser *parserE)
{
    free(parserE->tokens);
    free(parserE);
}


char** getTokens(Parser * parserE)
{
  if (parserE->tokens[0]==NULL) return NULL;

    return parserE->tokens;
}


int getNumTokens(Parser * parserE){
    return parserE->numtokens;
}