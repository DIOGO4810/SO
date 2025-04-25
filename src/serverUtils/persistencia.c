#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "dserver.h"

void writeDisco(Index* indice) {
    int fd = open("indexs", O_WRONLY | O_APPEND | O_CREAT, 0666);
    if (fd == -1) {
        perror("Erro ao abrir o ficheiro no write");
        return;
    }

    lseek(fd, 0, SEEK_END);

    write(fd,indice,getStructSize());
    
    close(fd);
}


Index* searchDisco(int ordem) {
    int fd = open("indexs", O_RDONLY);
    if (fd == -1) {
        perror("Erro ao abrir ficheiro no search");
        close(fd);
        return NULL;
    }

    Index* indice = malloc(getStructSize()); 
    if (indice == NULL) {
        perror("Erro ao alocar memória");
        close(fd);
        return NULL;
    }

    lseek(fd,(ordem-1)*getStructSize(),SEEK_SET);
    int bytesRead = read(fd,indice,getStructSize());
    if(bytesRead == 0 || getPidCliente(indice) == -1){
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
    if (!indice) {
        perror("Erro de alocação");
        close(fdW);
        close(fdR);
        return 1;
    }

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
    write(fdW, deleted, getStructSize());

    free(indice);
    free(deleted);
    close(fdW);
    close(fdR);

    return 0;
}