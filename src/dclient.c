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


void getServerMessage(char** argv,int fd){
    while (1)
    {
        fd = open("tmp/writeServerFIFO",O_RDONLY);
        if (fd == -1)continue;


        if(argv[1][1] == 'a'){

            char serverOutput[256] = "";
            (void)read(fd,serverOutput,256);
            printf("%s\n",serverOutput);
        }

        if(argv[1][1] == 'c'){

            char serverOutput[512] = "";
            (void)read(fd,serverOutput,512);

            Parser *parseFIFO = newParser();
            parseFIFO = parser(parseFIFO, serverOutput);
            char **tokens = getTokens(parseFIFO);
            if (atoi(tokens[0]) == 404){
                printf("Index não existente\n");
                freeParser(parseFIFO);
                break;
            }
            
            printf("Title: %s\nAuthors: %s\nYear: %d\nPath: %s\n",tokens[0],tokens[1],atoi(tokens[2]),tokens[3]);
            freeParser(parseFIFO);
        }


        break;
    }
}


int main(int argc, char *argv[])
{
    mkfifo(fifoDirectory,0666);
    int fd = -1;

    switch (argv[1][1])
    {
    case 'a':

        fd = open("tmp/writeClientFIFO",O_WRONLY);
        char *metaDados = concatInput(argc, argv, "%s %s %s %s %s", argv[1], argv[2], argv[3],argv[4],argv[5]);
        (void) write(fd,metaDados,strlen(metaDados));
        free(metaDados);
        close(fd);
        break;
    case 'c':
        fd = open("tmp/writeClientFIFO",O_WRONLY);
        char *indiceConsulta = concatInput(argc, argv, "%s %d ", argv[1], atoi(argv[2]));
        (void) write(fd,indiceConsulta,strlen(indiceConsulta));
        free(indiceConsulta);
        close(fd);
        
        break;
    case 'f':
        fd = open("tmp/writeClientFIFO",O_WRONLY);
        (void) write(fd,argv[1],strlen(argv[1]));
        close(fd);

        break;
    default:
        break;
    }


    if(argv[1][1] != 'f') getServerMessage(argv,fd);
    




    return 0;
}
