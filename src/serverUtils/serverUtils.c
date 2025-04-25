#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <glib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "dserver.h"
#include "parser.h"

void parseBufferToIntArray(char *buffer,GArray* ret) {

    Parser *parserObj = newParser(2048);
    parserObj = parser(parserObj, buffer, ' ');
    char **tokens = getTokens(parserObj);
    int size = getNumTokens(parserObj);

    for (int i = 0; i < size; i++) {
        if(i >0)if(strcmp(tokens[i],"End") == 0)break;
        int val = atoi(tokens[i]);
        g_array_append_val(ret, val);
    }

    freeParser(parserObj);
}



void findIndexsMatchParallel(GArray *ret, char *match, GArray *indexArray, int numProcesses)
{
    char *fifoPath = "tmp/matchFifo";

    if (access(fifoPath, F_OK) == 0) {
        unlink(fifoPath);
    }

    if (mkfifo(fifoPath, 0666) == -1)
    {
        perror("Erro ao criar FIFO");
        return;
    }

    int total = indexArray->len;
    int chunkSize = (total + numProcesses - 1) / numProcesses;


    pid_t pids[numProcesses];  

    for (int p = 0; p < numProcesses; p++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            int fifoWrite = open(fifoPath, O_WRONLY);

            if (fifoWrite == -1) exit(1);

            for (int i = p * chunkSize; i < (p + 1) * chunkSize && i < total; i++)
            {
                Index *indice = g_array_index(indexArray, Index *, i);

                char absoluteDirectory[256];
                char* path = getPath(indice);
                snprintf(absoluteDirectory, sizeof(absoluteDirectory), "Gdataset/%s", path);
                free(path);

                pid_t grepPid = fork();
                if (grepPid == 0)
                {
                    execl("/usr/bin/grep", "grep", "-q", "-w", match, absoluteDirectory, NULL);
                    // close(fifoWrite);
                    _exit(1);  // Se o execl falhar
                }

                int status;
                waitpid(grepPid, &status, 0);  
                if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
                {
                    char pidstr[64];

                    sprintf(pidstr,"%d ",getOrder(indice));
                    write(fifoWrite, pidstr, strlen(pidstr));
                }

            }

            close(fifoWrite);

            _exit(0);  
        }
        else if (pid > 0)
        {   

            pids[p] = pid;
        }
        else
        {
            perror("Erro ao criar o processo filho");
            exit(1);
        }
    }

        int fifoRead = open(fifoPath, O_RDONLY);
        int fifoWrite = open(fifoPath, O_WRONLY);
        

        // Espera todos os filhos terminarem
        for (int p = 0; p < numProcesses; p++)
        {
            int status;
            waitpid(pids[p], &status, 0);  // Espera pelo término de cada filho
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
            {
                printf("O processo filho %d terminou com erro\n", pids[p]);
            }
        }

        write(fifoWrite,"End ",4);

        if (fifoRead == -1)
        {
            perror("Erro ao abrir FIFO para leitura");
            unlink(fifoPath);
            return;
        }
    
        char buffer[16384];        printf("chegou\n");

        read(fifoRead, buffer, 16384);
        parseBufferToIntArray(buffer,ret);

    
        close(fifoRead);
        close(fifoWrite);

        unlink(fifoPath);
}
