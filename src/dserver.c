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

void freeIndex(Index *indice)
{
    free(indice);
}

void insert_index_into_table(GHashTable* cache,Index* indice ,int id) {
    int* key = malloc(sizeof(int));  // Aloca memória para a chave
    *key = id;
    g_hash_table_insert(cache, key, indice);
}

char* getPath(Index* indice){
    return strdup(indice->path);
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

    strcpy(deleted->title, "");
    strcpy(deleted->author, "");
    strcpy(deleted->path, "");
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
        freeIndex(item);
    }
    g_array_free(estrutura, TRUE);
}

void printIndice(Index *indice)
{
    printf(" Title: %s | Author: %s | Year: %d | Path: %s | PidCliente: %d\n",
           indice->title, indice->author, indice->year, indice->path,indice->pidCliente);
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
    sprintf(message, "%s|%s|%d|%s", indice->title, indice->author, indice->year, indice->path);
    (void)write(fdmessage, message, strlen(message));
    free(message);
    close(fdmessage);
}

void respondErrorMessage(char *diretoria)
{
    int error = mkfifo(diretoria, 0666);
    int fdmessage = open(diretoria, O_WRONLY);
    char *message = malloc(256 * sizeof(char));
    sprintf(message, "404");
    write(fdmessage, message, strlen(message));
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
    close(fdR);
    return nbytes;
}





void findIndexsMatch(GArray* ret, char* match,GArray* indexArray){
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
            execl("/usr/bin/grep", "grep", "-q", "-w", match, absoluteDirectory, NULL);
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
                    g_array_append_val(ret,indice->order);
                }
            }
        }
    }

    
}



GArray* getIndexsFromCacheAndDisc(GHashTable* cache) {
    GArray* indexArray = g_array_new(FALSE, FALSE, sizeof(Index*));

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, cache);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        g_array_append_val(indexArray, value); 
    }

    int fd = open("indexs", O_RDONLY | O_CREAT,0666);
    if (fd == -1) {
        perror("Erro ao abrir indexs");
        close(fd);
        return indexArray;
    }
    Index* indice = malloc(sizeof(Index));
    ssize_t bytesRead;
    while ((bytesRead = read(fd, indice, sizeof(Index))) == sizeof(Index)) {
        if (indice->pidCliente == -1) continue;
    
        if (!g_hash_table_contains(cache, indice)) {
            Index* copia = malloc(sizeof(Index));
            memcpy(copia, indice, sizeof(Index));
            g_array_append_val(indexArray, copia);
        }
    }
    free(indice);
    close(fd);
  
    return indexArray;
}






void handleInput(char **tokens, GHashTable* cache, int cacheSize,int* order)
{
    char diretoria[256] = "";

    switch (tokens[0][1])
    {
    case 'a':
    {
        (*order)++;
        Index *indice = createIndex(tokens + 1,*order);
        int pidCliente = atoi(tokens[5]);
        if(cacheSize > (int)g_hash_table_size(cache)){   
            insert_index_into_table(cache,indice,*order);            
        }
        
        writeDisco(indice);
        
        sprintf(diretoria, "tmp/writeServerFIFO%d",pidCliente);
        respondMessageAdiciona(diretoria,*order);
        

    }
    break;

    case 'c':
    {
        int pidCliente = atoi(tokens[2]);
        int pidBusca =  atoi(tokens[1]);
        sprintf(diretoria, "tmp/writeServerFIFO%d",pidCliente);

        Index* indice = g_hash_table_lookup(cache,&pidBusca);

        if (indice == NULL){
            indice = searchDisco(pidBusca);
            if(indice == NULL){
                respondErrorMessage(diretoria);
                break;
            }
            
        }
        respondMessageConsulta(diretoria,indice);

    }
    break;

    case 'd':
    {
        int pidCliente = atoi(tokens[2]);
        int pidBusca =  atoi(tokens[1]);
        sprintf(diretoria, "tmp/writeServerFIFO%d",pidCliente);
        int notFound = -1;
        Index* indice = g_hash_table_lookup(cache,&pidBusca);

        if (indice != NULL){
            g_hash_table_remove(cache,&pidBusca);
            
        }
        notFound = removeDisco(pidBusca);
        if(notFound == 1){
            respondErrorMessage(diretoria);
            break;
        }
        respondMessageRemove(diretoria,pidBusca);

    }
    break;
    case 'l':
    {
        int pidBusca = atoi(tokens[1]);
        sprintf(diretoria, "tmp/writeServerFIFO%d", atoi(tokens[3]));
        Index* indice = g_hash_table_lookup(cache,&pidBusca);
        
        if (indice == NULL){
            indice = searchDisco(pidBusca);
            if(indice == NULL){
                respondErrorMessage(diretoria);
                break;
            }

        }

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
        GArray *ret = g_array_new(FALSE, FALSE, sizeof(int));
        GArray* indexArray = getIndexsFromCacheAndDisc(cache);
        if(tokens[2][0] == 'n'){
            sprintf(diretoria, "tmp/writeServerFIFO%d", atoi(tokens[3]));
            findIndexsMatchParallel(ret,tokens[1],indexArray,atoi(tokens[2]+1));
        }else{
            sprintf(diretoria, "tmp/writeServerFIFO%d", atoi(tokens[2]));
            findIndexsMatch(ret,tokens[1],indexArray);
        }

        writeGArrayToFIFO(ret,diretoria);

        g_array_free(ret,TRUE);
        freeEstrutura(indexArray);
        break;
    }
    default:
        break;
    }
}




int main(int argc, char *argv[]) {
    GHashTable* cache = g_hash_table_new_full(g_int_hash, g_int_equal, free, (GDestroyNotify)freeIndex);
    int cacheSize = atoi(argv[2]);
    int fd;
    int ordem = 0;
    int fdOrdem = open("ordem",O_RDWR | O_CREAT,0666);
    read(fdOrdem,&ordem,sizeof(int));
    close(fdOrdem);
    int status;

    mkfifo("tmp/killzombies",0666);

    while (1) {
        fd = open("tmp/writeClientFIFO", O_RDONLY);
        
        if (fd == -1) {
            close(fd);
            continue;
        }

        char clientInput[1024] = "";
        char zombiepid[256] = "";

        read(fd, clientInput, 1024);
        printf("O cliente mandou isto:%s\n", clientInput);
        
        int nbytes = readDeadPid(zombiepid);
        
        if (nbytes > 0) {
            Parser *parseZombies = newParser();
            parseZombies = parser(parseZombies, zombiepid,' ');
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
                parseFIFO = parser(parseFIFO, clientInput,' ');
                char **tokens = getTokens(parseFIFO);
                handleInput(tokens, cache,cacheSize,&ordem);
                // sleep(5);
                writeDeadPid();
                freeParser(parseFIFO);
                exit(0);
            } else {
                continue;
            }
        }



        Parser *parseFIFO = newParser();
        parseFIFO = parser(parseFIFO, clientInput,'|');
        char **tokens = getTokens(parseFIFO);

        if (tokens[0][1] == 'f') {
            freeParser(parseFIFO);
            int fdOrdem = open("ordem",O_RDWR | O_CREAT,0666);
            write(fdOrdem,&ordem,sizeof(int));
            close(fdOrdem);
            close(fd);
            break;
        }

        handleInput(tokens, cache,cacheSize,&ordem);
        freeParser(parseFIFO);
        close(fd);
    }

    g_hash_table_destroy(cache);
    return 0;
}
