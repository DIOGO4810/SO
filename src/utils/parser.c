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
};


// Função que cria e aloca memoria para a estrutura e os campos do Parser
Parser *newParser()
{
    Parser *parserE = malloc(sizeof(struct parser));
    parserE->tokens = malloc(MaxTokensSize * sizeof(char *));
    parserE->line = NULL;
    return parserE;
}

Parser *parser(Parser *parserE,char* line)
{


    parserE->line = line;

    char *lineCopy = line;

    int i = 0;

    // Divide a linha em tokens usando strsep
    char *token = strsep(&lineCopy, " ");
    while (token != NULL && i < MaxTokensSize)
    {
        // Armazenar o token no array
        parserE->tokens[i++] = token;
        token = strsep(&lineCopy, " ");
    }

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