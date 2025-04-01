#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <glib.h>
#include "parser.h"

typedef struct index
{
    char *title;
    char *author;
    int year;
    char *path;
} Index;

Index *createIndex(char **tokens)
{
    Index *indice = malloc(sizeof(Index));
    indice->title = strdup(tokens[0]);
    indice->author = strdup(tokens[1]);
    indice->year = atoi(tokens[2]);
    indice->path = strdup(tokens[3]);

    return indice;
}

void freeIndex(Index *indice)
{
    free(indice->title);
    free(indice->author);
    free(indice->path);
    free(indice);
}

void freeEstrutura(GArray *estrutura)
{
    for (guint i = 0; i < estrutura->len; i++)
    {
        Index *item = g_array_index(estrutura, Index *, i);
        freeIndex(item);
    }
    g_array_free(estrutura, TRUE);
}

void printIndice(Index *indice)
{
    printf(" Title: %s | Author: %s | Year: %d | Path: %s\n",
           indice->title, indice->author, indice->year, indice->path);
}

void printGArrayIndex(GArray *array)
{
    printf("GArray contém %u elementos:\n", array->len);
    for (guint i = 0; i < array->len; i++)
    {
        Index *item = g_array_index(array, Index *, i);
        printIndice(item);
    }
}

void respondMessageAdiciona(char *diretoria, guint arraySize)
{
    mkfifo(diretoria, 0666);
    pid_t fdmessage = open(diretoria, O_WRONLY);
    char *message = malloc(256 * sizeof(char));
    sprintf(message, "Document %d indexed", (int)arraySize - 1);
    (void) write(fdmessage, message, strlen(message));
    close(fdmessage);
    free(message);
}

void respondMessageConsulta(char *diretoria,Index* indice)
{
    mkfifo(diretoria, 0666);
    pid_t fdmessage = open(diretoria, O_WRONLY);
    char *message = malloc(256 * sizeof(char));
    sprintf(message, "%s %s %d %s",indice->title, indice->author, indice->year, indice->path);
    (void) write(fdmessage, message, strlen(message));
    close(fdmessage);
    free(message);
}

void respondErrorMessage(char *diretoria)
{
    mkfifo(diretoria, 0666);
    pid_t fdmessage = open(diretoria, O_WRONLY);
    char *message = malloc(256 * sizeof(char));
    sprintf(message, "404");
    (void) write(fdmessage, message, strlen(message));
    close(fdmessage);
    free(message);
}



void handleInput(char **tokens,GArray* indexArray)
{
    

    switch (tokens[0][1])
    {
    case 'a':{

        Index *indice = createIndex(tokens + 1);
        g_array_append_val(indexArray, indice);
        respondMessageAdiciona("tmp/writeServerFIFO", indexArray->len);
    }
    break;

    case 'c':{
        int indiceArray = atoi(tokens[1]);
        if(indiceArray < 0 || indiceArray > (int)indexArray->len){
            respondErrorMessage("tmp/writeServerFIFO");
            break;
        }
        Index* indice = g_array_index(indexArray,Index*, indiceArray);
        respondMessageConsulta("tmp/writeServerFIFO",indice);

    }
    break;
    default:
        break;
    }
}

int main()
{

    GArray *indexArray = g_array_new(FALSE, FALSE, sizeof(Index *));
    while (1)
    {
        pid_t fd = open("tmp/writeClientFIFO", O_RDONLY);
        if (fd == -1)
        {
            continue;
        }

        char clientInput[512] = "";
        read(fd, clientInput, 512);
        printf("O cliente mandou isto:%s\n", clientInput);

        Parser *parseFIFO = newParser();
        parseFIFO = parser(parseFIFO, clientInput);
        char **tokens = getTokens(parseFIFO);

        if (tokens[0][1] == 'f')
        {
            freeParser(parseFIFO);
            close(fd);
            break;
        }

        handleInput(tokens,indexArray);

        freeParser(parseFIFO);
        close(fd);
    }

    printGArrayIndex(indexArray);

    freeEstrutura(indexArray);

    return 0;
}
