#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <glib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "dserver.h"
#include "parser.h"
#include "lruCache.h"

#define ZOMBIESFIFOPATH "tmp/killzombies"
#define CACHECONNECTPATH "tmp/cacheConnect"

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
        free(path);
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

    if (mkfifo(fifoPath, 0666) == -1)
    {
        perror("Erro ao criar FIFO");
        return;
    }

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
                free(path);
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
                    write(fifoWrite, pidstr, strlen(pidstr));
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
        int fifoWrite = open(fifoPath, O_WRONLY);
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

        write(fifoWrite,"End ",4);
        if (fifoRead == -1)
        {
            perror("Erro ao abrir FIFO para leitura");
            unlink(fifoPath);
            return;
        }
    
        char buffer[16384]; 
        read(fifoRead, buffer, 16384);
        parseBufferToIntArray(buffer,ret);

    
        close(fifoRead);
        close(fifoWrite);
        unlink(fifoPath);
}



void respondMessageAdiciona(char *diretoria, int indice)
{
    mkfifo(diretoria, 0666);
    int fdmessage = open(diretoria, O_WRONLY);
    char *message = malloc(256 * sizeof(char));
    sprintf(message, "Document %d indexed\n", indice);
    (void)write(fdmessage, message, strlen(message));
    free(message);
    close(fdmessage);
}



void respondMessageConsulta(char *diretoria, Index *indice)
{
    mkfifo(diretoria, 0666);
    int fdmessage = open(diretoria, O_WRONLY);
    char *message = malloc(512 * sizeof(char));
    
    char *title = getTitle(indice);
    char *author = getAuthor(indice);
    char *path = getPath(indice);

    sprintf(message, "%s|%s|%d|%s", title, author, getYear(indice), path);
    (void)write(fdmessage, message, strlen(message));

    free(title);
    free(author);
    free(path);
    free(message);
    close(fdmessage);
}




void respondMessageRemove(char *diretoria, int indice)
{
    mkfifo(diretoria, 0666);
    int fdmessage = open(diretoria, O_WRONLY);
    char *message = malloc(256 * sizeof(char));
    sprintf(message, "Index entry %d deleted\n", indice);
    (void)write(fdmessage, message, strlen(message));
    free(message);
    close(fdmessage);

}

void respondErrorMessage(char *diretoria)
{
    mkfifo(diretoria, 0666);
    int fdmessage = open(diretoria, O_WRONLY);
    char *message = malloc(256 * sizeof(char));
    sprintf(message, "404");
    write(fdmessage, message, strlen(message));

    free(message);
    close(fdmessage);
}

int checkAsync(char *input)
{
    if (input[1] == 'a' || input[1] == 'd' || input[1] == 'f')
        return 0;
    return 1 ;
}







void setupFIFOsAndDescriptors(int *fdRDdummyCache, int *fdRDdummyZombies) {
    mkfifo(ZOMBIESFIFOPATH, 0666);
    mkfifo(CACHECONNECTPATH, 0666);

    pid_t pid;
    if ((pid = fork()) == 0) {    
        int fdWriteZ = open(CACHECONNECTPATH, O_WRONLY);
        close(fdWriteZ);
        int fdWriteC = open(ZOMBIESFIFOPATH, O_WRONLY);
        close(fdWriteC);
        exit(0);
    } else {
        *fdRDdummyCache = open(CACHECONNECTPATH, O_RDONLY);
        *fdRDdummyZombies = open(ZOMBIESFIFOPATH, O_RDONLY);
    }
}



void cleanExit(int fdOrdem, int fd, int fdRDdummyCache, int fdRDdummyZombies, LRUCache *cacheLRU) {
    close(fdOrdem);
    close(fd);
    close(fdRDdummyCache);
    close(fdRDdummyZombies);
    lruCachePrint(cacheLRU);
}



void writeZombiePID() {
    char pidstr[16];
    sprintf(pidstr, "%d ", getpid());
    int fdWRZ = open(ZOMBIESFIFOPATH, O_WRONLY);
    write(fdWRZ, pidstr, strlen(pidstr));
    close(fdWRZ);
}

void writeCacheUpdate(Index* updateCache) {
    int fdWRC = open(CACHECONNECTPATH, O_WRONLY);
    if (fdWRC == -1) {
        perror("Erro a abrir CACHECONNECTPATH para escrita");
        return;
    }

    write(fdWRC, updateCache, getStructSize());
    close(fdWRC);
    free(updateCache);
}



void handleFIFOUpdates(int fdRDdummyZombies, int fdRDdummyCache, LRUCache *cacheLRU, int *status) {
    char zombiepid[256] = "";
    Index *cacheConnect = malloc(getStructSize());
    
    int nbytesZombies = read(fdRDdummyZombies, zombiepid, 256);
    int nbytesCache = read(fdRDdummyCache, cacheConnect, getStructSize());
    
    if (nbytesZombies > 0) {
        Parser *parseZombies = newParser(64);
        parseZombies = parser(parseZombies, zombiepid, ' ');
        char **tokens = getTokens(parseZombies);
        int size = getNumTokens(parseZombies);

        for (int i = 0; i < size; i++) {     
            int morto = waitpid(atoi(tokens[i]), status, 0);                  
            printf("Processo zombie morto: %d\n", morto);
        }
        freeParser(parseZombies);
    }

    if (nbytesCache > 0) {
        lruCachePut(cacheLRU, getOrder(cacheConnect), cacheConnect);
    }

    
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


