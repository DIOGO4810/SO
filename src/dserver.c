#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include<sys/wait.h>
#include <glib.h>
#include "parser.h"
#include "utils.h"

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
    int fdmessage = open(diretoria, O_WRONLY);
    char *message = malloc(256 * sizeof(char));
    sprintf(message, "Document %d indexed\n", (int)arraySize - 1);
    (void)write(fdmessage, message, strlen(message));
    close(fdmessage);
    free(message);
}

void respondMessageConsulta(char *diretoria, Index *indice)
{
    mkfifo(diretoria, 0666);
    int fdmessage = open(diretoria, O_WRONLY);
    char *message = malloc(256 * sizeof(char));
    sprintf(message, "%s %s %d %s", indice->title, indice->author, indice->year, indice->path);
    (void)write(fdmessage, message, strlen(message));
    close(fdmessage);
    free(message);
}

void respondErrorMessage(char *diretoria)
{
    mkfifo(diretoria, 0666);
    int fdmessage = open(diretoria, O_WRONLY);
    char *message = malloc(256 * sizeof(char));
    sprintf(message, "404");
    (void)write(fdmessage, message, strlen(message));
    close(fdmessage);
    free(message);
}

void respondMessageRemove(char *diretoria, int indice)
{
    mkfifo(diretoria, 0666);
    int fdmessage = open(diretoria, O_WRONLY);
    char *message = malloc(256 * sizeof(char));
    sprintf(message, "Index entry %d deleted\n", indice);
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
        if (g_array_index(indexArray, Index *, indiceArray) == NULL)
        {
            respondErrorMessage(diretoria);
            break;
        }
        freeIndex(g_array_index(indexArray, Index *, indiceArray));
        g_array_index(indexArray, Index *, indiceArray) = NULL;
        respondMessageRemove(diretoria, indiceArray);
    }
    break;
    case 'l':
    {
        int indiceArray = atoi(tokens[1]);
        sprintf(diretoria, "tmp/writeServerFIFO%d", atoi(tokens[3]));

        if (indiceArray < 0 || indiceArray > (int)indexArray->len)
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
        char absoluteDirectory[256] = "";
        sprintf(absoluteDirectory, "Gdataset/%s", indice->path);
        pid_t pid;
        mkfifo(diretoria, 0666);
        if ((pid = fork()) == 0)
        {
            int fdmessage = open(diretoria, O_WRONLY);
            dup2(fdmessage, 1);
            close(fdmessage);
            execl("/usr/bin/grep", "grep", "-c", "-w", tokens[2], absoluteDirectory, NULL);
            exit(1);
        }
    }
    break;

    case 's':
    {
        sprintf(diretoria, "tmp/writeServerFIFO%d", atoi(tokens[2]));
        GArray *ret = g_array_new(FALSE, FALSE, sizeof(int));
        for (guint i = 0; i < indexArray->len; i++)
        {
            if (g_array_index(indexArray, Index *, i) == NULL)
                continue;
            Index *indice = g_array_index(indexArray, Index *, i);
            char absoluteDirectory[256] = "";
            sprintf(absoluteDirectory, "Gdataset/%s", indice->path);
            pid_t pid;
            if ((pid = fork()) == 0)
            {
                execl("/usr/bin/grep", "grep", "-q", tokens[1], absoluteDirectory, NULL);
                exit(0);
            }
            else
            {
                int status;
                waitpid(pid, &status, 0);//Sincronizador do código assincrono

                if (WIFEXITED(status))
                {
                    int exit_status = WEXITSTATUS(status);
                    if (exit_status == 0)
                    {
                        g_array_append_val(ret, i);
                    }
                }
            }
        }
            
        writeGArrayToFIFO(ret,diretoria);
        break;
    }
    default:
        break;
    }
}

int checkAsync(char *input)
{
    if (input[1] == 'a' || input[1] == 'd' || input[1] == 'f')
        return 0;
    return 1 ;
}

void writeDeadPid(){
    char pidstr[10];
    sprintf(pidstr, "%d ", getpid());
    int fdWR = open("tmp/killzombies",O_RDWR);
    int fdW = open("tmp/killzombies",O_WRONLY);
    write(fdW, pidstr, strlen(pidstr));
    close(fdWR);
    close(fdW);

}


int readDeadPid(char* zombiepid){
    int fdWR = open("tmp/killzombies",O_RDWR);
    int fdW = open("tmp/killzombies",O_WRONLY);
    int fdR = open("tmp/killzombies",O_RDONLY);
    close(fdW);
    close(fdWR);
    int nbytes = read(fdR, zombiepid, 256);

    return nbytes;
}


int main() {
    GArray *indexArray = g_array_new(FALSE, FALSE, sizeof(Index *));
    int fd;

    int status;

    mkfifo("tmp/killzombies",0666);

    while (1) {
        fd = open("tmp/writeClientFIFO", O_RDONLY);
        
        if (fd == -1) {
            continue;
        }

        char clientInput[512] = "";
        char zombiepid[256] = "";

        read(fd, clientInput, 512);
        printf("O cliente mandou isto:%s\n", clientInput);
        
        int nbytes = readDeadPid(zombiepid);
        
        if (nbytes > 0) {
            Parser *parseZombies = newParser();
            parseZombies = parser(parseZombies, zombiepid);
            char **tokens = getTokens(parseZombies);
            int size = getNumTokens(parseZombies);

            for (int i = 0; i <size ; i++){     
                int morto = waitpid(atoi(tokens[i]), &status, 0);                  
                printf("Processo zombie morto:%d\n",morto);
            }
            
        }

        

        int isAsync = checkAsync(clientInput);
        if (isAsync) {
            pid_t pid;
            if ((pid = fork()) == 0) {    
     
                Parser *parseFIFO = newParser();
                parseFIFO = parser(parseFIFO, clientInput);
                char **tokens = getTokens(parseFIFO);
                handleInput(tokens, indexArray);
                sleep(5);
                writeDeadPid();

                freeParser(parseFIFO);
                exit(0);
            } else {
                continue;
            }
        }



        Parser *parseFIFO = newParser();
        parseFIFO = parser(parseFIFO, clientInput);
        char **tokens = getTokens(parseFIFO);

        if (tokens[0][1] == 'f') {
            freeParser(parseFIFO);
            close(fd);
            break;
        }

        handleInput(tokens, indexArray);
        freeParser(parseFIFO);
    }

    close(fd);
    printGArrayIndex(indexArray);
    freeEstrutura(indexArray);

    return 0;
}
