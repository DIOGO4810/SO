#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <glib.h>
#include "parser.h"
#include "utils.h"
#include "persistencia.h"
#include "serverUtils.h"
#include "lruCache.h"
#include "index.h"

#define fifoDirectory "tmp/writeClientFIFO"

void spawnGrepCount(char *datasetDirectory, char *diretoria, char *match, Index *indice)
{
    char absoluteDirectory[256];
    sprintf(absoluteDirectory, "%s/%s", datasetDirectory, getPath(indice));

    if (fork() == 0)
    {
        int fdmessage = open(diretoria, O_WRONLY);
        dup2(fdmessage, 1);
        close(fdmessage);
        execl("/usr/bin/grep", "grep", "-c", "-w", match, absoluteDirectory, NULL);
        exit(1);
    }
}

void handleInputSync(Index* receivedMessage, LRUCache *cache, int *order)
{
    char diretoria[256] = "";

    int pidCliente = getPidCliente(receivedMessage);  

    switch (getMessageType(receivedMessage))  
    {
    case 'a':
    {
        (*order)++;
        setOrder(receivedMessage, *order);  // Usando setter
        lruCachePut(cache, *order, receivedMessage);
        writeDisco(receivedMessage);
        sprintf(diretoria, "tmp/writeServerFIFO%d", pidCliente);
        respondMessageAdiciona(diretoria, *order);
        break;
    }

    case 'd':
    {
        int keyBusca = getKey(receivedMessage);  
        sprintf(diretoria, "tmp/writeServerFIFO%d", pidCliente);
        lruCacheRemove(cache, keyBusca);

        if (removeDisco(keyBusca) == 1)
        {
            respondErrorMessage(diretoria);
            break;
        }
        respondMessageRemove(diretoria, keyBusca);
        break;
    }

    default:
        break;
    }
}

Index *handleInputAsync(char *datasetDirectory, Index *receivedMessage, LRUCache *cache)
{
    char diretoria[256] = "";
    int keyBusca = getKey(receivedMessage);  
    int pidCliente = getPidCliente(receivedMessage);  

    switch (getMessageType(receivedMessage))  
    {
    case 'c':
    {
        sprintf(diretoria, "tmp/writeServerFIFO%d", pidCliente);
        Index *indice = lruCacheGet(cache, keyBusca);

        if (indice == NULL)
        {
            indice = searchDisco(keyBusca);
            if (indice == NULL)
            {
                respondErrorMessage(diretoria);
                return getDeletedIndex();
            }
            respondMessageConsulta(diretoria, indice);
            return indice;
        }
        respondMessageConsulta(diretoria, indice);
        return indice;
    }

    case 'l':
    {
        sprintf(diretoria, "tmp/writeServerFIFO%d", pidCliente);
        mkfifo(diretoria, 0666);
        Index *indice = lruCacheGet(cache, keyBusca);

        if (indice == NULL)
        {
            indice = searchDisco(keyBusca);
            if (indice == NULL)
            {
                respondErrorMessage(diretoria);
                return getDeletedIndex();
            }
            spawnGrepCount(datasetDirectory, diretoria, getKeyWord(receivedMessage), indice);  
            return indice;
        }
        spawnGrepCount(datasetDirectory, diretoria, getKeyWord(receivedMessage), indice);  
        return indice;
    }

    case 's':
    {
        GArray *ret = g_array_new(FALSE, FALSE, sizeof(int));
        GArray *indexArray = getIndexsFromCacheAndDisc(cache);

        if (getKeyWord(receivedMessage)[0] == 'n')  
        {
            sprintf(diretoria, "tmp/writeServerFIFO%d", pidCliente);
            findIndexsMatchParallel(datasetDirectory, ret, getKeyWord(receivedMessage)+1, indexArray, getNumProcessos(receivedMessage));  
        }
        else
        {
            sprintf(diretoria, "tmp/writeServerFIFO%d", pidCliente); printf("dnasoda\n");
            findIndexsMatch(datasetDirectory, ret, getKeyWord(receivedMessage), indexArray);
        }
       
        writeGArrayToFIFO(ret, diretoria);
        g_array_free(ret, TRUE);
        freeEstrutura(indexArray);
        return getDeletedIndex();
    }
    default:
        break;
    }

    return getDeletedIndex();
}

void initializeServer(int argc, char *argv[], int *cacheSize, LRUCache **cacheLRU, int *ordem, int *fdOrdem)
{
    printf("Server pid: %d\n", getpid());
    (void)argc;
    *cacheSize = atoi(argv[2]);
    *cacheLRU = lruCacheNew(*cacheSize);
    cachePopulateDisco(*cacheSize, *cacheLRU);

    *fdOrdem = open("ordem", O_RDONLY | O_CREAT, 0666);
    read(*fdOrdem, ordem, sizeof(int));
    close(*fdOrdem);
}

int main(int argc, char *argv[])
{
    int cacheSize;
    int ordem = 0;
    int status;
    int fdOrdem;
    LRUCache *cacheLRU;
    int fd;

    initializeServer(argc, argv, &cacheSize, &cacheLRU, &ordem, &fdOrdem);

    while (1)
    {
        fd = open(fifoDirectory, O_RDONLY);

        if (fd == -1)
        {
            continue;
        }

        Index* receivedMessage = malloc(getStructSize());  
        read(fd, receivedMessage, getStructSize());
        printIndice(receivedMessage);
        int updated = handleFIFOUpdates(cacheLRU, &status, receivedMessage);
        if(updated){
            close(fd);
            continue;
        }
        int isAsync = checkAsync(getMessageType(receivedMessage));  
        if (isAsync)
        {
            pid_t pid;
            close(fd);
            if ((pid = fork()) == 0)
            {
                Index *sonUpdate = handleInputAsync(argv[1], receivedMessage, cacheLRU);
                
                setPidZombie(sonUpdate, getpid());  

                writeSonUpdate(sonUpdate);
                 
                exit(0);
            }
            else
            {
                continue;
            }
        }

        if (getMessageType(receivedMessage) == 'f')  
        {
            fdOrdem = open("ordem", O_WRONLY | O_CREAT, 0666);
            write(fdOrdem, &ordem, sizeof(int));
            cleanExit(fdOrdem, fd, cacheLRU);
            break;
        }

        handleInputSync(receivedMessage, cacheLRU, &ordem);
        close(fd);
    }

    lruCacheFree(cacheLRU);
    return 0;
}
