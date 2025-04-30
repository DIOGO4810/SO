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
#include "dserver.h"
#include "serverUtils.h"
#include "lruCache.h"

#define fifoDirectory "tmp/writeClientFIFO"

struct index
{
    char title[250];
    char author[100];
    int year;
    char path[100];
    int pidCliente;
    int order;
};

Index *createIndex(char **tokens,int order)
{
    Index *indice = malloc(sizeof(Index));
    if (indice == NULL) return NULL; 

    strncpy(indice->title, tokens[0], sizeof(indice->title) - 1);
    indice->title[sizeof(indice->title) - 1] = '\0';

    strncpy(indice->author, tokens[1], sizeof(indice->author) - 1);
    indice->author[sizeof(indice->author) - 1] = '\0';

    indice->year = atoi(tokens[2]);

    strncpy(indice->path, tokens[3], sizeof(indice->path) - 1);
    indice->path[sizeof(indice->path) - 1] = '\0';

    indice->pidCliente = atoi(tokens[4]);

    indice->order = order;

    return indice;
}



void insert_index_into_table(GHashTable* cache,Index* indice ,int id) {
    int* key = malloc(sizeof(int));  // Aloca memória para a chave
    *key = id;
    g_hash_table_insert(cache, key, indice);
}

char* getPath(Index* indice){
    return strdup(indice->path);
}

char* getTitle(Index* indice){
    return strdup(indice->title);
}

char* getAuthor(Index* indice){
    return strdup(indice->author);
}

int getYear (Index* indice){
    return indice->year;
}

int getPidCliente(Index* indice){
    return indice->pidCliente;
}

int getStructSize(){
    return sizeof(Index);
}

int getOrder(Index* indice){
    return indice->order;
}

Index* getDeletedIndex() {
    Index* deleted = malloc(sizeof(Index));
    if (!deleted) return NULL; 

    strcpy(deleted->title, "\0");
    strcpy(deleted->author, "\0");
    strcpy(deleted->path, "\0");
    deleted->year = 0;
    deleted->order = -1;
    deleted->pidCliente = -1;

    return deleted;
}



void freeEstrutura(GArray *estrutura)
{
    for (guint i = 0; i < estrutura->len; i++)
    {
        if (g_array_index(estrutura, Index *, i) == NULL)
            continue;
        Index *item = g_array_index(estrutura, Index *, i);
        free(item);
    }
    g_array_free(estrutura, TRUE);
}

void printIndice(Index *indice)
{
    if(indice == NULL)printf("Indice é Nulo");
    printf(" Title: %s | Author: %s | Year: %d | Path: %s | PidCliente: %d\n",
           indice->title, indice->author, indice->year, indice->path,indice->pidCliente);
}


void spawnGrepCount( char *datasetDirectory,  char *diretoria, char *match, Index *indice) {
    char absoluteDirectory[256];
    sprintf(absoluteDirectory, "%s/%s", datasetDirectory, indice->path);

    if (fork() == 0) {
        int fdmessage = open(diretoria, O_WRONLY);
        dup2(fdmessage, 1);
        close(fdmessage);
        execl("/usr/bin/grep", "grep", "-c", "-w", match, absoluteDirectory, NULL);
        exit(1);
    }
}








void handleInputSync(char **tokens, LRUCache* cache, int* order)
{
    char diretoria[256] = "";

    switch (tokens[0][1])
    {
    case 'a':  
    {
        (*order)++;
        Index *indice = createIndex(tokens + 1, *order);
        int pidCliente = atoi(tokens[5]);

        lruCachePut(cache, *order, indice);
        writeDisco(indice);

        sprintf(diretoria, "tmp/writeServerFIFO%d", pidCliente);
        respondMessageAdiciona(diretoria, *order);
        break;
    }

    case 'd':
    {
        int pidCliente = atoi(tokens[2]);
        int pidBusca = atoi(tokens[1]);

        sprintf(diretoria, "tmp/writeServerFIFO%d", pidCliente);
        lruCacheRemove(cache, pidBusca);

        if (removeDisco(pidBusca) == 1) {
            respondErrorMessage(diretoria);
            break;
        }
        respondMessageRemove(diretoria, pidBusca);
        break;
    }

    default:
        break;
    }
}





Index* handleInputAsync(char* datasetDirectory, char **tokens, LRUCache* cache)
{
    char diretoria[256] = "";
    switch (tokens[0][1])
    {
        case 'c':  
        {
            int pidCliente = atoi(tokens[2]);
            int pidBusca = atoi(tokens[1]);
            sprintf(diretoria, "tmp/writeServerFIFO%d", pidCliente);
            Index* indice = lruCacheGet(cache, pidBusca);
        
            if (indice == NULL) {
                indice = searchDisco(pidBusca);
                if (indice == NULL) {
                    respondErrorMessage(diretoria);
                    return NULL;
                }
                respondMessageConsulta(diretoria, indice);
                return indice;  
            }
            respondMessageConsulta(diretoria, indice);
            return indice;
        }
        
        case 'l': 
        {
            int pidBusca = atoi(tokens[1]);
            int pidCliente = atoi(tokens[3]);
            sprintf(diretoria, "tmp/writeServerFIFO%d", pidCliente);
            mkfifo(diretoria, 0666);
            Index* indice = lruCacheGet(cache, pidBusca);
            
            if (indice == NULL) {
                indice = searchDisco(pidBusca);
                if (indice == NULL) {
                    respondErrorMessage(diretoria);
                    return NULL;
                }
                spawnGrepCount(datasetDirectory, diretoria, tokens[2], indice);
                return indice;
            }
            spawnGrepCount(datasetDirectory, diretoria, tokens[2], indice);
            return indice;
        }
        

        case 's':  
        {
            GArray *ret = g_array_new(FALSE, FALSE, sizeof(int));
            GArray* indexArray = getIndexsFromCacheAndDisc(cache);
            if (tokens[2][0] == 'n') {
                sprintf(diretoria, "tmp/writeServerFIFO%d", atoi(tokens[3]));
                findIndexsMatchParallel(datasetDirectory, ret, tokens[1], indexArray, atoi(tokens[2] + 1));
            } else {
                sprintf(diretoria, "tmp/writeServerFIFO%d", atoi(tokens[2]));
                findIndexsMatch(datasetDirectory, ret, tokens[1], indexArray);
            }
            writeGArrayToFIFO(ret, diretoria);
            g_array_free(ret, TRUE);
            freeEstrutura(indexArray);
            return NULL;
        }
        default:
            break;
    }
    return NULL;
}


void initializeServer(int argc, char *argv[], int *cacheSize, LRUCache **cacheLRU, int *ordem, int *fdOrdem) {
    printf("Server pid: %d\n", getpid());
    (void)argc;
    *cacheSize = atoi(argv[2]);
    *cacheLRU = lruCacheNew(*cacheSize);
    cachePopulateDisco(*cacheSize, *cacheLRU);

    *fdOrdem = open("ordem", O_RDONLY | O_CREAT, 0666);
    read(*fdOrdem, ordem, sizeof(int));
    close(*fdOrdem);
}




int main(int argc, char *argv[]) {
    int cacheSize;
    int ordem = 0;
    int status;
    int fdRDdummyCache;
    int fdRDdummyZombies;
    int fdOrdem;
    LRUCache* cacheLRU;
    int fd;
    
    initializeServer(argc, argv, &cacheSize, &cacheLRU, &ordem, &fdOrdem);
    setupFIFOsAndDescriptors(&fdRDdummyCache, &fdRDdummyZombies);

    while (1) {
        fd = open(fifoDirectory, O_RDONLY);
        
        if (fd == -1) {
            continue;
        }

        char clientInput[1024] = "";
        read(fd, clientInput, 1024);
        printf("O cliente mandou isto: %s\n", clientInput);
        
        handleFIFOUpdates(fdRDdummyZombies, fdRDdummyCache, cacheLRU, &status);

        int isAsync = checkAsync(clientInput);
        if (isAsync) {
            pid_t pid;
            close(fd);
            if ((pid = fork()) == 0) {    
                Parser *parseFIFO = newParser(10);
                parseFIFO = parser(parseFIFO, clientInput, ' ');
                char **tokens = getTokens(parseFIFO);
                Index* updateCache = handleInputAsync(argv[1], tokens, cacheLRU);

                writeZombiePID();

                if (updateCache != NULL) {
                    writeCacheUpdate(updateCache);
                }
                freeParser(parseFIFO);
                exit(0);
            } else {
                continue;
            }
        }

        Parser *parseFIFO = newParser(10);
        parseFIFO = parser(parseFIFO, clientInput, '|');
        char **tokens = getTokens(parseFIFO);

        if (tokens[0][1] == 'f') {
            freeParser(parseFIFO);
            fdOrdem = open("ordem", O_WRONLY | O_CREAT, 0666);
            write(fdOrdem, &ordem, sizeof(int));
            cleanExit(fdOrdem, fd, fdRDdummyCache, fdRDdummyZombies, cacheLRU);
            break;
        }

        handleInputSync(tokens, cacheLRU, &ordem);
        freeParser(parseFIFO);
        close(fd);
    }

    close(fdRDdummyCache);
    close(fdRDdummyZombies);
    lruCacheFree(cacheLRU);
    return 0;
}
