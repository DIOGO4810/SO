#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
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
        if (g_array_index(estrutura, Index *, i) == NULL)
            continue;
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
    int numeroValidos = 0;
    for (guint i = 0; i < array->len; i++)
    {
        if (g_array_index(array, Index *, i) == NULL)
            continue;
        numeroValidos++;
        Index *item = g_array_index(array, Index *, i);
        printIndice(item);
    }
    printf("GArray contém %d elementos\n", numeroValidos);
}

void respondMessageAdiciona(char *diretoria, guint arraySize)
{
    mkfifo(diretoria, 0666);
    pid_t fdmessage = open(diretoria, O_WRONLY);
    char *message = malloc(256 * sizeof(char));
    sprintf(message, "Document %d indexed", (int)arraySize - 1);
    (void)write(fdmessage, message, strlen(message));
    close(fdmessage);
    free(message);
}

void respondMessageConsulta(char *diretoria, Index *indice)
{
    mkfifo(diretoria, 0666);
    pid_t fdmessage = open(diretoria, O_WRONLY);
    char *message = malloc(256 * sizeof(char));
    sprintf(message, "%s %s %d %s", indice->title, indice->author, indice->year, indice->path);
    (void)write(fdmessage, message, strlen(message));
    close(fdmessage);
    free(message);
}

void respondErrorMessage(char *diretoria)
{
    mkfifo(diretoria, 0666);
    pid_t fdmessage = open(diretoria, O_WRONLY);
    char *message = malloc(256 * sizeof(char));
    sprintf(message, "404");
    (void)write(fdmessage, message, strlen(message));
    close(fdmessage);
    free(message);
}

void respondMessageRemove(char *diretoria, int indice)
{
    mkfifo(diretoria, 0666);
    pid_t fdmessage = open(diretoria, O_WRONLY);
    char *message = malloc(256 * sizeof(char));
    sprintf(message, "Index entry %d deleted", indice);
    (void)write(fdmessage, message, strlen(message));
    close(fdmessage);
    free(message);
}

void handleInput(char **tokens, GArray *indexArray)
{
    char diretoria[256] = "";

    switch (tokens[0][1])
    {
    case 'a':
    {

        Index *indice = createIndex(tokens + 1);
        g_array_append_val(indexArray, indice);
        sprintf(diretoria, "tmp/writeServerFIFO%d", atoi(tokens[5]));
        respondMessageAdiciona(diretoria, indexArray->len);
    }
    break;

    case 'c':
    {
        int indiceArray = atoi(tokens[1]);
        sprintf(diretoria, "tmp/writeServerFIFO%d", atoi(tokens[2]));

        if (indexArray->len == 0 || indiceArray < 0 || indiceArray > (int)indexArray->len)
        {
            respondErrorMessage(diretoria);
            break;
        }
        if (g_array_index(indexArray, Index *, indiceArray) == NULL)
        {
            respondErrorMessage(diretoria);
            break;
        }
        Index *indice = g_array_index(indexArray, Index *, indiceArray);
        respondMessageConsulta(diretoria, indice);
    }
    break;

    case 'd':
    {
        int indiceArray = atoi(tokens[1]);
        sprintf(diretoria, "tmp/writeServerFIFO%d", atoi(tokens[2]));

        if (indiceArray < 0 || indiceArray > (int)indexArray->len)
        {
            respondErrorMessage(diretoria);
            break;
        }
        freeIndex(g_array_index(indexArray, Index *, indiceArray));
        g_array_index(indexArray, Index *, indiceArray) = NULL;
        respondMessageRemove(diretoria, indiceArray);
    }
    break;
    // case 'l':
    // {
    //     int indiceArray = atoi(tokens[1]);
    //     sprintf(diretoria, "tmp/writeServerFIFO%d", atoi(tokens[2]));

    //     if (indiceArray < 0 || indiceArray > (int)indexArray->len)
    //     {
    //         respondErrorMessage(diretoria);
    //         break;
    //     }
    //     Index* indice = g_array_index(indexArray, Index *, indiceArray);
    // }
    // break;
    default:
        break;
    }
}

int checkAsync(char *input)
{
    if (input[1] == 'a' || input[1] == 'd' || input[1] == 'f')
        return 0;
    return 1;
}

int main()
{
    // struct sigaction sa;
    // sa.sa_handler = SIG_IGN;  
    // sigaction(SIGCHLD, &sa, NULL);

    

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

        int isAsync = checkAsync(clientInput);
        if (isAsync)
        {
            pid_t pid;
            if ((pid = fork()) == 0)
            {
                Parser *parseFIFO = newParser();
                parseFIFO = parser(parseFIFO, clientInput);
                char **tokens = getTokens(parseFIFO);

                handleInput(tokens, indexArray);

                freeParser(parseFIFO);
                exit(0);
            }
            else
                continue;
        }

        Parser *parseFIFO = newParser();
        parseFIFO = parser(parseFIFO, clientInput);
        char **tokens = getTokens(parseFIFO);

        if (tokens[0][1] == 'f')
        {
            freeParser(parseFIFO);
            close(fd);
            break;
        }

        handleInput(tokens, indexArray);

        freeParser(parseFIFO);
        close(fd);
    }

    printGArrayIndex(indexArray);

    freeEstrutura(indexArray);

    return 0;
}