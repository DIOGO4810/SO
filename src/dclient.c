#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "utils.h"
#include "parser.h"
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
    char *message = NULL;

    switch (argv[1][1]) {
        case 'a':
            if (!validaInput(argv)) return 0;
            message = concatInput(argc, argv, "%s|%s|%s|%s|%s|%d", argv[1], argv[2], argv[3], argv[4], argv[5], getpid());
            break;
        case 'c':
            message = concatInput(argc, argv, "%s %d %d ", argv[1], atoi(argv[2]), getpid());
            break;
        case 'd':
            message = concatInput(argc, argv, "%s|%d|%d", argv[1], atoi(argv[2]), getpid());
            break;
        case 'f':
            writeToFIFO(fifoDirectory, argv[1]);
            break;
        case 'l':
            message = concatInput(argc, argv, "%s %d %s %d ", argv[1], atoi(argv[2]), argv[3], getpid());
            break;
        case 's':
            if (argc == 4) {
                message = concatInput(argc, argv, "%s %s n%s %d ", argv[1], argv[2], argv[3], getpid());
            } else {
                message = concatInput(argc, argv, "%s %s %d ", argv[1], argv[2], getpid());
            }
            break;
        default:
            break;
    }

    if (message != NULL) {
        fd = open(fifoDirectory, O_WRONLY);
        if (fd == -1) {
            perror("Erro ao abrir FIFO para escrita");
            free(message);
            return 0;
        }
        write(fd, message, strlen(message));
        free(message);
        close(fd);
    }

    if (argv[1][1] != 'f') {
        getServerMessage(argv);  
    }

    return 0;
}
