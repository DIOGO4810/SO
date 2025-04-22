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

void getServerMessage(char **argv, int fd)
{
    pid_t mypid = getpid();
    char diretoria[256];
    sprintf(diretoria, "tmp/writeServerFIFO%d", mypid);
    while (1)
    {

        fd = open(diretoria, O_RDONLY);
        if (fd == -1)
            continue;

        char command = argv[1][1];

        switch (command)
        {
        case 'a':
        {
            char serverOutput[256] = "";
            (void)read(fd, serverOutput, 256);

            write(1, serverOutput, strlen(serverOutput)); 
            break;
        }

        case 'c':
        {
            char serverOutput[512] = "";
            (void)read(fd, serverOutput, 512);

            Parser *parseFIFO = newParser();
            parseFIFO = parser(parseFIFO, serverOutput);
            char **tokens = getTokens(parseFIFO);

            if (atoi(tokens[0]) == 404)
            {
                write(1, "Index não existente\n", 21); 
                freeParser(parseFIFO);
                break;
            }

            char output[512];
            snprintf(output, sizeof(output), "Title: %s\nAuthors: %s\nYear: %d\nPath: %s\n",
                     tokens[0], tokens[1], atoi(tokens[2]), tokens[3]);
            write(1, output, strlen(output)); 

            freeParser(parseFIFO);
            break;
        }

        case 'd':
        {
            char serverOutput[256] = "";
            (void)read(fd, serverOutput, 256);
            if (atoi(serverOutput) == 404)
            {
                write(1, "Index não existente\n", 21); 
                break;
            }
            write(1, serverOutput, strlen(serverOutput)); 
            break;
        }

        case 'l':
        {
            char serverOutput[256] = "";
            (void)read(fd, serverOutput, 256);
            if (atoi(serverOutput) == 404)
            {
                write(1, "Index não existente\n", 21); 
                break;
            }
            write(1, serverOutput, strlen(serverOutput)); 
            break;
        }

        case 's':
        {
            char serverOutput[1024] = "";
            (void)read(fd, serverOutput, 1024);
            write(1, serverOutput, strlen(serverOutput)); 
            break;
        }
        default:
            printf("Comando inválido\n");
            break;
        }
        break;
    }
}

int main(int argc, char *argv[])
{
    mkfifo(fifoDirectory, 0666);
    int fd = -1;

    switch (argv[1][1])
    {
    case 'a':
        if(!validaInput(argv))return 0;
        fd = open("tmp/writeClientFIFO", O_WRONLY);
        char *metaDados = concatInput(argc, argv, "%s %s %s %s %s %d ", argv[1], argv[2], argv[3], argv[4], argv[5], getpid());
        (void)write(fd, metaDados, strlen(metaDados));
        free(metaDados);
        close(fd);
        break;
    case 'c':
        fd = open("tmp/writeClientFIFO", O_WRONLY);
        char *indiceConsulta = concatInput(argc, argv, "%s %d %d ", argv[1], atoi(argv[2]), getpid());
        (void)write(fd, indiceConsulta, strlen(indiceConsulta));
        free(indiceConsulta);
        close(fd);

        break;

    case 'd':
        fd = open("tmp/writeClientFIFO", O_WRONLY);
        char *indiceRemove = concatInput(argc, argv, "%s %d %d ", argv[1], atoi(argv[2]), getpid());
        (void)write(fd, indiceRemove, strlen(indiceRemove));
        free(indiceRemove);
        close(fd);

        break;
    case 'f':
        fd = open("tmp/writeClientFIFO", O_WRONLY);
        (void)write(fd, argv[1], strlen(argv[1]));
        close(fd);
        
        break;
    case 'l':
        fd = open("tmp/writeClientFIFO", O_WRONLY);
        char *indicelinhas = concatInput(argc, argv, "%s %d %s %d ", argv[1], atoi(argv[2]), argv[3], getpid());
        (void)write(fd, indicelinhas, strlen(indicelinhas));
        close(fd);

        break;
    case 's':
        fd = open("tmp/writeClientFIFO", O_WRONLY);
        char *indiceSearch = concatInput(argc, argv, "%s %s %d ", argv[1], argv[2], getpid());
        (void)write(fd, indiceSearch, strlen(indiceSearch));
        close(fd);

        break;
    default:
        break;
    }

    if (argv[1][1] != 'f')
        getServerMessage(argv, fd);

    return 0;
}
