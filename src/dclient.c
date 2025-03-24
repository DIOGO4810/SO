#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h> 
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>    
#include "utils.h"
#include <glib.h>

#define fifoDirectory "tmp/writeClientFIFO"


int main(int argc, char *argv[])
{
    mkfifo(fifoDirectory,0666);
    int fd;

    switch (argv[1][1])
    {
    case 'a':

        fd = open("tmp/writeClientFIFO",O_WRONLY);
        char* metaDados = concatInput(argc,argv);
        write(fd,metaDados,strlen(metaDados));
        free(metaDados);
        close(fd);
        break;
    case 'f':
        fd = open("tmp/writeClientFIFO",O_WRONLY);
        write(fd,argv[1],strlen(argv[1]));
        close(fd);

        break;
    default:
        break;
    }




    return 0;
}
