#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>


int main(int argc, char const *argv[])
{   
    char clientInput[512];
    while (1)
    {
        int fd = open("tmp/writeClientFIFO",O_RDONLY);
        if(fd == -1){
            sleep(1);
            continue;
        }
        int nBytes = read(fd,clientInput,512);
        printf("O cliente mandou isto:%s\n",clientInput);
    }
    

    return 0;
}
