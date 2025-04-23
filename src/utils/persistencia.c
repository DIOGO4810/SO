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
    if(bytesRead > 0)return indice;

    close(fd);
    free(indice);  
    return NULL;
}




int removeCsvLine(int pidCliente) {
    int fd = open("indexs.csv", O_RDWR);
    if (fd == -1) {
        perror("Erro ao abrir o ficheiro");
        return -1;
    }

    char pidStr[16];
    snprintf(pidStr, sizeof(pidStr), "%d", pidCliente);

    char buffer[4096];
    ssize_t bytesRead = read(fd, buffer, 4096 - 1);
    if (bytesRead < 0) {
        perror("Erro ao ler o ficheiro");
        close(fd);
        return -1;
    }
    buffer[bytesRead] = '\0';

    // Encontra a linha a remover
    char* output = malloc(4096);
    if (!output) {
        perror("Erro de memória");
        close(fd);
        return -1;
    }

    char* saveptr;
    char* linha = strtok_r(buffer, "\n", &saveptr);
    ssize_t newLen = 0;
    int found = 0;

    while (linha != NULL) {
        if (strstr(linha, pidStr) == NULL) {
            ssize_t len = snprintf(output + newLen, 4096 - newLen, "%s\n", linha);
            newLen += len;
        }else {
            found = 1;
        }
        linha = strtok_r(NULL, "\n", &saveptr);
    }
    if (found == 0)return 1;
    // Volta ao início, escreve novo conteúdo, e corta o ficheiro
    lseek(fd, 0, SEEK_SET);
    write(fd, output, newLen);
    ftruncate(fd, newLen);

    free(output);
    close(fd);
    return 0;
}