#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h> 
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>    
#include "utils.h"

#define fifoDirectory "tmp/writeClientFIFO"

int main(int argc, char *argv[])
{

    mkfifo(fifoDirectory,0666);

    switch (argv[1][1])
    {
    case 'a':

        int fd = open("tmp/writeClientFIFO",O_WRONLY);
        char* metaDados = concatInput(argc,argv);
        write(fd,metaDados,strlen(metaDados));
        break;
    
    default:
        break;
    }




    return 0;
}
