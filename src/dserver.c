#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <glib.h>
#include "parser.h"

typedef struct index
{
    char* title;
    char* author;
    int year;
    char* path;
}Index;



Index* createIndex(char** tokens){
    Index* indice = malloc (sizeof(Index));
    indice->title = strdup(tokens[0]);
    indice->author = strdup(tokens[1]);
    indice->year = atoi(tokens[2]);
    indice->path = strdup(tokens[3]);

    return indice;
}

void freeIndex(Index* indice){
    free(indice->title);
    free(indice->author);
    free(indice->path);
    free(indice);
}

void insert_albums_into_table(GHashTable* indexTable,char* path,Index* indice) {
    g_hash_table_insert(indexTable, strdup(path), indice);
}

void lookup_index(GHashTable* indexTable, const char* path) {
    Index* indice = g_hash_table_lookup(indexTable, path);
    
    if (indice != NULL) {
        printf("Index encontrado para o caminho: %s\n", path);
        printf("Título: %s\n", indice->title);
        printf("Autor: %s\n", indice->author);
        printf("Ano: %d\n", indice->year);
        printf("Caminho: %s\n", indice->path);
    } else {
        printf("Index não encontrado para o caminho: %s\n", path);
    }
}


int main()
{   

    GHashTable* indexTable = g_hash_table_new_full(g_str_hash,g_str_equal, free, (GDestroyNotify)freeIndex);
    while (1)
    {
        char clientInput[512] = "";
        pid_t fd = open("tmp/writeClientFIFO",O_RDONLY);

        if(fd == -1){
            continue;
        }

        int nBytes = read(fd,clientInput,512);
        close(fd);
        
        Parser* parseFIFO = newParser();

        parseFIFO = parser(parseFIFO,clientInput);

        char** tokens = getTokens(parseFIFO);
        if(tokens[0][1] == 'f'){
            printf("O cliente mandou isto:%s\n",clientInput);
            freeParser(parseFIFO);
            break;
        }
        Index* indice = createIndex(tokens + 1);
        insert_albums_into_table(indexTable,tokens[4],indice);


        lookup_index(indexTable,tokens[4]);

        printf("O cliente mandou isto:%s\n",clientInput);
        freeParser(parseFIFO);
    }
    
    g_hash_table_destroy(indexTable);
    

    return 0;
}
