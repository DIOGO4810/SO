#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "index.h"
#include "lruCache.h"

void writeDisco(Index* indice) {
    int fd = open("indexs", O_WRONLY | O_APPEND | O_CREAT, 0666);
    if (fd == -1) {
        perror("Erro ao abrir o ficheiro no write");
        return;
    }

    if (lseek(fd, 0, SEEK_END) == -1) {
        perror("Erro no lseek");
        close(fd);
        return;
    }

    ssize_t bytes = write(fd, indice, getStructSize());
    if (bytes != getStructSize()) {
        perror("Erro ao escrever no ficheiro");
    }

    close(fd);
}



Index* searchDisco(int ordem) {
    int fd = open("indexs", O_RDONLY);
    if (fd == -1) {
        perror("Erro ao abrir ficheiro no search");
        return NULL;
    }

    Index* indice = malloc(getStructSize()); 

    if (lseek(fd, (ordem - 1) * getStructSize(), SEEK_SET) == -1) {
        perror("Erro no lseek");
        free(indice);
        close(fd);
        return NULL;
    }

    int bytesRead = read(fd, indice, getStructSize());
    if (bytesRead != getStructSize() || getOrder(indice) == -1) {
        free(indice); 
        close(fd);
        return NULL;
    }
    
    close(fd);
    return indice;
}






int removeDisco(int ordem) {
    int fdW = open("indexs", O_WRONLY);
    int fdR = open("indexs", O_RDONLY);


    Index* indice = malloc(getStructSize());

    off_t offset = (ordem - 1) * getStructSize();
    lseek(fdR, offset, SEEK_SET);

    int bytesRead = read(fdR, indice, getStructSize());
    if (bytesRead <= 0) {
        free(indice);
        close(fdW);
        close(fdR);
        return 1;  // Linha não existe
    }
    
    if(getOrder(indice) == -1){
        free(indice);
        close(fdW);
        close(fdR);
        return 1;  // Indice está eliminado
    }
    // Substituir por uma struct marcada como deletada
    Index* deleted = getDeletedIndex();
    lseek(fdW, offset, SEEK_SET);
    ssize_t writtenDeleted = write(fdW, deleted, getStructSize());
    if(writtenDeleted < 0){
        perror("Erro ao escrever no ficheiro o indice Eliminado");
    }

    free(indice);
    free(deleted);
    close(fdW);
    close(fdR);

    return 0;
}




void cachePopulateDisco(int cacheSize, LRUCache* cache) {
    int fdR = open("indexs", O_RDONLY | O_CREAT, 0666);
    if (fdR == -1) {
        perror("Erro ao abrir ficheiro para preencher cache");
        return;
    }

    for (int i = 0; i < cacheSize;) {  
        Index* indice = malloc(getStructSize());

        int bytesRead = read(fdR, indice, getStructSize());
        if (bytesRead != getStructSize()) {
            free(indice);  
            break;  
        }
        
        if (getOrder(indice) == -1) {
            free(indice);  
            continue; 
        }

        printIndice(indice);
        lruCachePut(cache, getOrder(indice), indice);
        i++;  
    }

    close(fdR);
}





GArray* getIndexsFromCacheAndDisc(LRUCache* cache) {
    GArray* indexArray = lruCacheFill(cache);

    int fd = open("indexs", O_RDONLY | O_CREAT, 0666);
    if (fd == -1) {
        perror("Erro ao abrir indexs");
        return indexArray;
    }

    Index* indice = malloc(getStructSize());
    ssize_t bytesRead;

    while ((bytesRead = read(fd, indice, getStructSize())) == getStructSize()) {
        if (getPidCliente(indice) == -1) continue;
        if (!lruCacheContains(cache, indice)) {
            Index* copia = malloc(getStructSize());
            memcpy(copia, indice, getStructSize());
            g_array_append_val(indexArray, copia);
        }
    }

    free(indice);
    close(fd);
    
    return indexArray;
}

