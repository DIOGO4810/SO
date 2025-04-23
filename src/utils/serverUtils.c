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

void findIndexsMatchParallel(GArray *ret, char *match, GArray *indexArray, int numProcesses)
{
    char *fifoPath = "tmp/matchFifo";

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
        int fifoWrite = open(fifoPath, O_RDWR);
        pid_t pid = fork();
        if (pid == 0)
        {
            if (fifoWrite == -1)
                exit(1);

            for (int i = p * chunkSize; i < (p + 1) * chunkSize && i < total; i++)
            {
                Index *indice = g_array_index(indexArray, Index *, i);

                char absoluteDirectory[256];
                snprintf(absoluteDirectory, sizeof(absoluteDirectory), "Gdataset/%s", getPath(indice));

                pid_t grepPid = fork();
                if (grepPid == 0)
                {
                    execl("/usr/bin/grep", "grep", "-q", "-w", match, absoluteDirectory, NULL);
                    _exit(1);  // Se o execl falhar
                }

                int status;
                waitpid(grepPid, &status, 0);  
                if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
                {
                    int pidCliente = getPidCliente(indice);
                    char pidstr[64];

                    sprintf(pidstr,"%d ",pidCliente);
                    int wstat =  write(fifoWrite, pidstr, strlen(pidstr));
                    printf("write status: %d\n",wstat);
                }
            }

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


    printf("dpasd\n");
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

        printf("mmmmmmmmm\n");

    int fifoRead = open(fifoPath, O_RDWR);
        if (fifoRead == -1)
        {
            perror("Erro ao abrir FIFO para leitura");
            unlink(fifoPath);
            return;
        }
    
        char buffer[4096];
        read(fifoRead, buffer, 4096);
        printf("PID Cliente com match: %s\n", buffer);
    
        close(fifoRead);

        unlink(fifoPath);
}
