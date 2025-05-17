#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <glib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "index.h"
#include "parser.h"
#include "lruCache.h"

#define fifoDirectory "tmp/writeClientFIFO"


void parseBufferToIntArray(char *buffer,GArray* ret) {

    Parser *parserObj = newParser(2048);
    parserObj = parser(parserObj, buffer, ' ');
    char **tokens = getTokens(parserObj);
    int size = getNumTokens(parserObj);

    for (int i = 0; i < size; i++) {
        if(i >0)if(strcmp(tokens[i],"End") == 0)break;
        int val = atoi(tokens[i]);
        g_array_append_val(ret, val);
    }

    freeParser(parserObj);
}


void findIndexsMatch(char* datasetDirectory,GArray* ret, char* match,GArray* indexArray){
    for (guint i = 0; i < indexArray->len; i++)
    {
        if (g_array_index(indexArray, Index *, i) == NULL)
            continue;
        Index *indice = g_array_index(indexArray, Index *, i);
        char absoluteDirectory[256] = "";
        char* path = getPath(indice);
        sprintf(absoluteDirectory, "%s/%s",datasetDirectory,path );
        
        pid_t pid;
        if ((pid = fork()) == 0)
        {
            execl("/usr/bin/grep", "grep", "-q", "-w", match, absoluteDirectory, NULL);
            exit(0);
        }
        else
        {
            int status;
            waitpid(pid, &status, 0);//Sincronizador do código assincrono

            if (WIFEXITED(status)) {
                int exit_status = WEXITSTATUS(status);
                if (exit_status == 0) {
                    int order = getOrder(indice);
                    g_array_append_val(ret, order);
                }
            }            
        }
    }

    
}



void findIndexsMatchParallel(char* datasetDirectory,GArray *ret, char *match, GArray *indexArray, int numProcesses)
{    
    char *fifoPath = "tmp/matchFifo";
    if (access(fifoPath, F_OK) == 0) {
        unlink(fifoPath);
    }

    mkfifo(fifoPath, 0666);
    

    int total = indexArray->len;
    int chunkSize = (total + numProcesses - 1) / numProcesses;
    pid_t pids[numProcesses];  

    for (int p = 0; p < numProcesses; p++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            int fifoWrite = open(fifoPath, O_WRONLY);
            if (fifoWrite == -1) exit(1);

            for (int i = p * chunkSize; i < (p + 1) * chunkSize && i < total; i++)
            {
                Index *indice = g_array_index(indexArray, Index *, i);
                char absoluteDirectory[256];
                char* path = getPath(indice);
                snprintf(absoluteDirectory, sizeof(absoluteDirectory), "%s/%s",datasetDirectory, path);
                pid_t grepPid = fork();
                if (grepPid == 0)
                {
                    execl("/usr/bin/grep", "grep", "-q", "-w", match, absoluteDirectory, NULL);
                    _exit(1);  // Se o execl falhar
                }

                int status;
                waitpid(grepPid, &status, 0);  
                if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
                {
                    char pidstr[64];
                    sprintf(pidstr,"%d ",getOrder(indice));
                    ssize_t written = write(fifoWrite, pidstr, strlen(pidstr));
                    if (written == -1) {
                        perror("Erro na escrita");
                    }
                }

            }
            close(fifoWrite);
            _exit(0);  
        }
        else if (pid > 0)
        {   
            pids[p] = pid;
        }
        else
        {
            perror("Erro ao criar o processo filho");
            exit(1);
        }
    }

        int fifoRead = open(fifoPath, O_RDONLY);   
        if (fifoRead == -1)
        {
            perror("Erro ao abrir FIFO para leitura");
            unlink(fifoPath);
            return;
        }
        int fifoWrite = open(fifoPath, O_WRONLY);
        if (fifoWrite == -1)
        {
            perror("Erro ao abrir FIFO para escrita");
            unlink(fifoPath);
            return;
        }
        // Espera todos os filhos terminarem
        for (int p = 0; p < numProcesses; p++)
        {
            int status;
            waitpid(pids[p], &status, 0);  // Espera pelo término de cada filho
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
            {
                printf("O processo filho %d terminou com erro\n", pids[p]);
            }
        }

        ssize_t writtenEnd = write(fifoWrite,"End ",4);
        if (writtenEnd == -1) {
            perror("Erro na escrita");
        }
    
        char buffer[16384]; 
        ssize_t bytesRead = read(fifoRead, buffer, 16384);
        if (bytesRead == -1) {
            perror("Erro na leitura do FIFO (Buffer)");
        }
        parseBufferToIntArray(buffer,ret);

    
        close(fifoRead);
        close(fifoWrite);
        unlink(fifoPath);
}



void respondMessageAdiciona(char *diretoria, int indice) {
    mkfifo(diretoria,0666);

    int fdmessage = open(diretoria, O_WRONLY);
    if (fdmessage == -1) {
        perror("Erro ao abrir FIFO para escrita");
        return;
    }

    char *message = malloc(256 * sizeof(char));

    snprintf(message, 256, "Document %d indexed\n", indice);

    if (write(fdmessage, message, strlen(message)) == -1) {
        perror("Erro ao escrever na FIFO");
    }

    free(message);

    if (close(fdmessage) == -1) {
        perror("Erro ao fechar FIFO");
    }
}



void respondMessageConsulta(char *diretoria, Index *indice) {
    mkfifo(diretoria,0666);

    int fdmessage = open(diretoria, O_WRONLY);
    if (fdmessage == -1) {
        perror("Erro ao abrir FIFO para escrita");
        return;
    }

    char *message = malloc(512 * sizeof(char));

    char *title = getTitle(indice);
    char *author = getAuthor(indice);
    char *path = getPath(indice);

    snprintf(message, 512, "%s|%s|%d|%s", title, author, getYear(indice), path);

    if (write(fdmessage, message, strlen(message)) == -1) {
        perror("Erro ao escrever na FIFO");
    }

    free(message);

    if (close(fdmessage) == -1) {
        perror("Erro ao fechar FIFO");
    }
}





void respondMessageRemove(char *diretoria, int indice) {
    mkfifo(diretoria,0666);

    int fdmessage = open(diretoria, O_WRONLY);
    if (fdmessage == -1) {
        perror("Erro ao abrir FIFO para escrita");
        return;
    }

    char *message = malloc(256 * sizeof(char));

    snprintf(message, 256, "Index entry %d deleted\n", indice);

    if (write(fdmessage, message, strlen(message)) == -1) {
        perror("Erro ao escrever na FIFO");
    }

    free(message);

    if (close(fdmessage) == -1) {
        perror("Erro ao fechar FIFO");
    }
}


void respondErrorMessage(char *diretoria) {
    mkfifo(diretoria,0666);

    int fdmessage = open(diretoria, O_WRONLY);
    if (fdmessage == -1) {
        perror("Erro ao abrir FIFO para escrita");
        return;
    }

    const char *message = "404";

    if (write(fdmessage, message, strlen(message)) == -1) {
        perror("Erro ao escrever na FIFO");
    }

    if (close(fdmessage) == -1) {
        perror("Erro ao fechar FIFO");
    }
}


int checkAsync(char type)
{
    if (type == 'a' || type == 'd' || type == 'f')
        return 0;
    return 1 ;
}





void cleanExit(int fdOrdem, int fd ,LRUCache *cacheLRU) {
    close(fdOrdem);
    close(fd);

    lruCachePrint(cacheLRU);
}



void writeSonUpdate(Index* sonUpdate) {
    int fdWRC = open(fifoDirectory, O_WRONLY);
    if (fdWRC == -1) {
        perror("Erro a abrir fifo para escrita");
        return;
    }

    ssize_t writtenSonUpdate = write(fdWRC, sonUpdate, getStructSize());
    if (writtenSonUpdate == -1) {
        perror("Erro ao escrever na FIFO");
        close(fdWRC);
        return;
    }
    close(fdWRC);
    if(getOrder(sonUpdate) == -1)free(sonUpdate);
}



int handleFIFOUpdates(LRUCache *cacheLRU, int *status, Index *receivedMessage) {
    if (getPidZombie(receivedMessage)== -1) {
        return 0;
    }

    int pidZombie = getPidZombie(receivedMessage);
    waitpid(pidZombie, status, 0);
    printf("Processo zombie for eliminado: %d\n",pidZombie);
    if (getOrder(receivedMessage) == -1) {
        return 1;
    }

    Index *novaEntrada = malloc(getStructSize());

    // Copia os dados de receivedMessage para novaEntrada
    memcpy(novaEntrada, receivedMessage, getStructSize());

    // Coloca a nova entrada na cache LRU
    lruCachePut(cacheLRU, getOrder(novaEntrada), novaEntrada);
    return 1;
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


