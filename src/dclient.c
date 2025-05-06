#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "utils.h"
#include "parser.h"
#include "index.h"
#include <glib.h>

#define fifoDirectory "tmp/writeClientFIFO"


void getServerMessage(char **argv) {
    pid_t mypid = getpid();
    char diretoria[256];
    sprintf(diretoria, "tmp/writeServerFIFO%d", mypid);

    while (1) {
        int fd = open(diretoria, O_RDONLY);
        if (fd == -1) {
            continue;
        }
        char smallBuffer[256] = "";
        char *bigBuffer = NULL;

        if (argv[1][1] == 's') {
            bigBuffer = malloc(16384);  
            (void)read(fd, bigBuffer, 16384);
        } else {
            (void)read(fd, smallBuffer, sizeof(smallBuffer)); 
        }

        if (atoi(smallBuffer) == 404) {
            write(1, "Index não existente\n", 21);
            close(fd);
            unlink(diretoria);
            break;
        }

        char command = argv[1][1];
        switch (command) {
            case 'a':
            case 'd':
            case 'l':
                write(1, smallBuffer, strlen(smallBuffer));  
                break;

            case 's':
                write(1, bigBuffer, strlen(bigBuffer));  
                break;

            case 'c': {
                Parser *parseFIFO = newParser(10);
                parseFIFO = parser(parseFIFO, smallBuffer, '|');
                char **tokens = getTokens(parseFIFO);
                char output[512];
                snprintf(output, sizeof(output), "Title: %s\nAuthors: %s\nYear: %d\nPath: %s\n", tokens[0], tokens[1], atoi(tokens[2]), tokens[3]);
                write(1, output, strlen(output));
                freeParser(parseFIFO);
                break;
            }

            default:
                write(1, "Comando inválido\n", 17);
                break;
        }

        close(fd);
        unlink(diretoria);
        if (bigBuffer != NULL)free(bigBuffer); 
        break;
    }
}



void writeToFIFO(char *fifoPath, char *message) {
    int fd = open(fifoPath, O_WRONLY);
    if (fd == -1) {
        perror("Erro ao abrir FIFO para escrita");
        return;
    }
    write(fd, message, strlen(message));
    close(fd);
}


int main(int argc, char *argv[]) {
    mkfifo(fifoDirectory, 0666);  

    int fd = -1;
    Index* message = malloc(getStructSize());
    if (!message) {
        perror("Erro ao alocar memória para message");
        return 1;
    }

    // Inicialização comum
    setPidZombie(message, -1);
    setOrder(message, -1);
    setTitle(message, "");
    setAuthor(message, "");
    setPath(message, "");
    setKeyWord(message, "");
    setYear(message, 0);
    setKey(message, -1);
    setMessageType(message, '\0');
    setPidCliente(message, getpid());     
    setNumProcessos(message,-1);

    switch (argv[1][1]) {
        case 'a':  // adicionar
            if (!validaInput(argv)) {
                free(message);
                return 0;
            }
            setTitle(message, argv[2]);
            setAuthor(message, argv[3]);
            setYear(message, atoi(argv[4]));
            setPath(message, argv[5]);
            setMessageType(message, 'a');
            break;

        case 'c':  // consultar
            setKey(message, atoi(argv[2]));
            setMessageType(message, 'c');
            break;

        case 'd':  // deletar
            setKey(message, atoi(argv[2]));
            setMessageType(message, 'd');
            break;

        case 'f':  // finalizar
            setMessageType(message, 'f');
            setPidCliente(message, -1); 
            break;

        case 'l':  // localizar
            setKey(message, atoi(argv[2]));
            setKeyWord(message, argv[3]);
            setMessageType(message, 'l');
            break;

        case 's':  // search
            setMessageType(message, 's');
            if (argc == 4) {
                char keywordFull[30];
                snprintf(keywordFull, sizeof(keywordFull), "n%s", argv[2]);
                setKeyWord(message, keywordFull);
                setNumProcessos(message,atoi(argv[3]));
            } else {
                setKeyWord(message, argv[2]);
            }
            break;

        default:
            printf("Comando inválido.\n");
            free(message);
            return 0;
    }

    // Escreve a estrutura Index na FIFO
    fd = open(fifoDirectory, O_WRONLY);
    if (fd == -1) {
        perror("Erro ao abrir FIFO para escrita");
        free(message);
        return 0;
    }

    write(fd, message,getStructSize());
    close(fd);
    free(message);

    // Aguarda resposta do servidor, exceto no comando -f
    if (argv[1][1] != 'f') {
        getServerMessage(argv);
    }

    return 0;
}

