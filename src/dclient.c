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

void safeWrite(int fd, const char *buffer, size_t length) {

        ssize_t written = write(fd, buffer , length );
        if (written == -1) {
            perror("Erro na escrita");
        }
}

void getServerMessage(char **argv) {
    pid_t mypid = getpid();
    char diretoria[256];
    snprintf(diretoria, sizeof(diretoria), "tmp/writeServerFIFO%d", mypid);

    while (1) {
        int fd = open(diretoria, O_RDONLY);
        if (fd == -1) {
            continue;
        }

        char smallBuffer[256] = "";
        char *bigBuffer = NULL;
        ssize_t bytesRead;

        if (argv[1][1] == 's') {
            bigBuffer = malloc(16384);
            if (!bigBuffer) {
                perror("Erro ao alocar memória para bigBuffer");
                close(fd);
                break;
            }
            bytesRead = read(fd, bigBuffer, 16384 - 1);
            if (bytesRead == -1) {
                perror("Erro na leitura do FIFO (bigBuffer)");
                free(bigBuffer);
                close(fd);
                break;
            }
            bigBuffer[bytesRead] = '\0';
        } else {
            bytesRead = read(fd, smallBuffer, sizeof(smallBuffer) - 1);
            if (bytesRead == -1) {
                perror("Erro na leitura do FIFO (smallBuffer)");
                close(fd);
                break;
            }
            smallBuffer[bytesRead] = '\0';
        }

        if (atoi(smallBuffer) == 404) {
            safeWrite(STDOUT_FILENO, "Index não existente\n", 21);
            close(fd);
            if (unlink(diretoria) == -1) {
                perror("Erro a apagar FIFO");
            }
            break;
        }

        char command = argv[1][1];
        switch (command) {
            case 'a':
            case 'd':
            case 'l':
                safeWrite(STDOUT_FILENO, smallBuffer, strlen(smallBuffer));
                break;

            case 's':
                safeWrite(STDOUT_FILENO, bigBuffer, strlen(bigBuffer));
                break;

            case 'c': {
                Parser *parseFIFO = newParser(10);
                if (!parseFIFO) {
                    fprintf(stderr, "Erro ao criar parser\n");
                    break;
                }
                parseFIFO = parser(parseFIFO, smallBuffer, '|');
                char **tokens = getTokens(parseFIFO);
                char output[512];
                snprintf(output, sizeof(output), "Title: %s\nAuthors: %s\nYear: %d\nPath: %s\n",
                         tokens[0], tokens[1], atoi(tokens[2]), tokens[3]);
                safeWrite(STDOUT_FILENO, output, strlen(output));
                freeParser(parseFIFO);
                break;
            }

            default:
                safeWrite(STDOUT_FILENO, "Comando inválido\n", 17);
                break;
        }

        close(fd);
        if (unlink(diretoria) == -1) {
            perror("Erro a apagar FIFO");
        }
        if (bigBuffer != NULL) free(bigBuffer);
        break;
    }
}

void writeToFIFO(char *fifoPath, char *message) {
    int fd = open(fifoPath, O_WRONLY);
    if (fd == -1) {
        perror("Erro ao abrir FIFO para escrita");
        return;
    }
    safeWrite(fd, message, strlen(message));
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
    setNumProcessos(message, -1);

    if (argc < 2) {
        fprintf(stderr, "Uso: %s -[a|c|d|f|l|s] [args]\n", argv[0]);
        free(message);
        return 1;
    }

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
                setNumProcessos(message, atoi(argv[3]));
            } else {
                setKeyWord(message, argv[2]);
            }
            break;

        default:
            fprintf(stderr, "Comando inválido.\n");
            free(message);
            return 1;
    }

    fd = open(fifoDirectory, O_WRONLY);
    if (fd == -1) {
        perror("Erro ao abrir FIFO para escrita");
        free(message);
        return 1;
    }

    if (write(fd, message, getStructSize()) == -1) {
        perror("Erro na escrita para FIFO");
        close(fd);
        free(message);
        return 1;
    }

    close(fd);
    free(message);

    if (argv[1][1] != 'f') {
        getServerMessage(argv);
    }

    return 0;
}
