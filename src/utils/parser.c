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
    parserE->tokens = malloc(MaxTokensSize * 200);    
    parserE->line = NULL;
    parserE->numtokens = 0;
    return parserE;
}

Parser *parser(Parser *parserE, char *line, char limitador) {
    if (!parserE || !line) return NULL;

    // Guardar a linha original
    parserE->line = line;

    // Criar uma cópia da linha porque strsep modifica a string
    char *lineCopy = strdup(line);
    if (!lineCopy) return NULL;

    // Construir o string de delimitador
    char delimStr[2] = { limitador, '\0' };

    int i = 0;
    char *token = strsep(&lineCopy, delimStr);
    while (token != NULL && i < MaxTokensSize) {
        parserE->tokens[i++] = token;
        token = strsep(&lineCopy, delimStr);
    }

    parserE->numtokens = i;

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
    return parserE->numtokens-1;
}

void printTokens(char **tokens) {
    if (tokens == NULL) {
        printf("Lista de tokens é NULL.\n");
        return;
    }

    int i = 0;
    while (tokens[i] != NULL ) {
        printf("Token %d: %s\n", i, tokens[i]);
        i++;
    }
}
