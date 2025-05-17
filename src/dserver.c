#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
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
    snprintf(absoluteDirectory, sizeof(absoluteDirectory), "%s/%s", datasetDirectory, getPath(indice));

    pid_t pid = fork();
    if (pid < 0) {
        perror("Erro no fork em spawnGrepCount");
        return;
    }
    if (pid == 0)
    {
        int fdmessage = open(diretoria, O_WRONLY);
        if (fdmessage == -1) {
            perror("Erro ao abrir FIFO para escrita no spawnGrepCount");
            exit(EXIT_FAILURE);
        }
        if (dup2(fdmessage, STDOUT_FILENO) == -1) {
            perror("Erro no dup2 em spawnGrepCount");
            close(fdmessage);
            exit(EXIT_FAILURE);
        }
        close(fdmessage);

        execl("/usr/bin/grep", "grep", "-c", "-w", match, absoluteDirectory, NULL);
        perror("Erro ao executar grep em spawnGrepCount");
        exit(EXIT_FAILURE);
    }
    // O processo pai continua sem esperar pelo filho (conforme lógica original)
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
        snprintf(diretoria, sizeof(diretoria), "tmp/writeServerFIFO%d", pidCliente);
        respondMessageAdiciona(diretoria, *order);
        break;
    }

    case 'd':
    {
        int keyBusca = getKey(receivedMessage);
        snprintf(diretoria, sizeof(diretoria), "tmp/writeServerFIFO%d", pidCliente);
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
        snprintf(diretoria, sizeof(diretoria), "tmp/writeServerFIFO%d", pidCliente);
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
        snprintf(diretoria, sizeof(diretoria), "tmp/writeServerFIFO%d", pidCliente);
        if (mkfifo(diretoria, 0666) == -1 ) {
            perror("Erro ao criar FIFO no handleInputAsync caso 'l'");
            respondErrorMessage(diretoria);
            return getDeletedIndex();
        }
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
        if (!ret) {
            fprintf(stderr, "Erro ao criar GArray em handleInputAsync\n");
            return getDeletedIndex();
        }
        GArray *indexArray = getIndexsFromCacheAndDisc(cache);
        if (!indexArray) {
            fprintf(stderr, "Erro ao obter índices em handleInputAsync\n");
            g_array_free(ret, TRUE);
            return getDeletedIndex();
        }

        snprintf(diretoria, sizeof(diretoria), "tmp/writeServerFIFO%d", pidCliente);

        if (getKeyWord(receivedMessage)[0] == 'n')  // Busca paralela
        {
            findIndexsMatchParallel(datasetDirectory, ret, getKeyWord(receivedMessage) + 1, indexArray, getNumProcessos(receivedMessage));
        }
        else
        {
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
    if (*fdOrdem == -1) {
        perror("Erro ao abrir ficheiro ordem");
        exit(EXIT_FAILURE);
    }
    ssize_t bytesRead = read(*fdOrdem, ordem, sizeof(int));
    if (bytesRead == -1) {
        perror("Erro ao ler ficheiro ordem");
        close(*fdOrdem);
        exit(EXIT_FAILURE);
    }
    close(*fdOrdem);
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Usa: %s <dataset_directory> <cache_size>\n", argv[0]);
        return EXIT_FAILURE;
    }

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
        if (fd == -1){
            continue;
        }

        Index* receivedMessage = malloc(getStructSize());
        if (!receivedMessage) {
            perror("Erro ao alocar memória para receivedMessage");
            close(fd);
            continue;
        }

        ssize_t bytesRead = read(fd, receivedMessage, getStructSize());
        if (bytesRead == -1) {
            perror("Erro na leitura do FIFO principal");
            free(receivedMessage);
            close(fd);
            continue;
        } 

        printIndice(receivedMessage);

        int updated = handleFIFOUpdates(cacheLRU, &status, receivedMessage);
        if (updated) {
            free(receivedMessage);
            close(fd);
            continue;
        }

        int isAsync = checkAsync(getMessageType(receivedMessage));
        if (isAsync)
        {
            pid_t pid;
            close(fd);

            if ((pid = fork()) == 0){
                Index *sonUpdate = handleInputAsync(argv[1], receivedMessage, cacheLRU);

                setPidZombie(sonUpdate, getpid());

                writeSonUpdate(sonUpdate);

                free(receivedMessage);
                exit(EXIT_SUCCESS);
            }
            else
            {
                free(receivedMessage);
                continue;
            }
        }

        if (getMessageType(receivedMessage) == 'f')  // Finalizar servidor
        {
            fdOrdem = open("ordem", O_WRONLY | O_CREAT, 0666);
            if (fdOrdem == -1) {
                perror("Erro ao abrir ficheiro ordem para escrita");
                free(receivedMessage);
                close(fd);
                break;
            }
            if (write(fdOrdem, &ordem, sizeof(int)) == -1) {
                perror("Erro ao escrever ficheiro ordem");
            }
            cleanExit(fdOrdem, fd, cacheLRU);
            break;
        }

        handleInputSync(receivedMessage, cacheLRU, &ordem);
        close(fd);
    }

    lruCacheFree(cacheLRU);
    return 0;
}
