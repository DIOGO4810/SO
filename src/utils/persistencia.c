#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "dserver.h"

void writeDisco(Index* indice) {
    int fd = open("indexs", O_WRONLY | O_APPEND | O_CREAT, 0666);
    if (fd == -1) {
        perror("Erro ao abrir o ficheiro");
        return;
    }

    lseek(fd, 0, SEEK_END);

    write(fd,indice,getStructSize());
    
    close(fd);
}


Index* searchDisco(int ordem) {
    int fd = open("indexs", O_RDONLY);
    if (fd == -1) {
        perror("Erro ao abrir ficheiro");
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
        return NULL;
    }

    close(fd);
    return indice;
}




int removeDisco(int ordem) {
    int fd = open("indexs", O_RDWR);
    if (fd == -1) {
        perror("Erro ao abrir o ficheiro");
        return -1;
    }

    Index* indice = malloc(getStructSize());
    if (!indice) {
        perror("Erro de alocação");
        close(fd);
        return -1;
    }

    off_t offset = (ordem - 1) * getStructSize();
    lseek(fd, offset, SEEK_SET);

    int bytesRead = read(fd, indice, getStructSize());
    if (bytesRead <= 0) {
        free(indice);
        close(fd);
        return 1;  // Linha não existe
    }

    // Substituir por uma struct marcada como deletada
    Index* deleted = getDeletedIndex();
    lseek(fd, offset, SEEK_SET);
    write(fd, deleted, getStructSize());

    free(indice);
    free(deleted);
    close(fd);

    return 0;
}