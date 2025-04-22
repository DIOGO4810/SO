#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

void writeCsv(char** tokens) {
    int fd = open("indexs.csv", O_WRONLY | O_APPEND | O_CREAT, 0666);
    if (fd == -1) {
        perror("Erro ao abrir o ficheiro");
        return;
    }

    lseek(fd, 0, SEEK_END);

    int wroteSomething = 0;

    for (int i = 1; tokens[i] != NULL; i++) {
        if (tokens[i][0] == '\0') continue; // ignora tokens vazios

        if (wroteSomething) {
            write(fd, ";", 1); // só escreve ; se já escreveu algo antes
        }

        write(fd, tokens[i], strlen(tokens[i]));
        wroteSomething = 1;
    }

    write(fd, "\n", 1);
    close(fd);
}


char* searchCsv(int pidCliente) {
    int fd = open("indexs.csv", O_RDONLY);
    if (fd == -1) {
        perror("Erro ao abrir ficheiro");
        return NULL;
    }

    // Tento ler o ficheiro inteiro de uma vez
    char buffer[4096];
    ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
    if (bytesRead <= 0) {
        close(fd);
        return NULL;  
    }
    buffer[bytesRead] = '\0'; 

   
    char pidStr[16];
    snprintf(pidStr, sizeof(pidStr), "%d", pidCliente);

    // Divide o ficheiro lido em linhas
    char* linha = strtok(buffer, "\n");
    while (linha != NULL) {
        if (strstr(linha, pidStr) != NULL) {
            close(fd);
            return strdup(linha);  
        }
        linha = strtok(NULL, "\n");  // Next line
    }

    
    close(fd);
    return NULL;
}