#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>
#include <glib.h>
#include "dserver.h"

char *removeAspas(char *str)
{
    size_t len = strlen(str);

    // Verifica se a string tem pelo menos duas aspas e se começa e termina com aspas
    if (len >= 2 && str[0] == '"' && str[len - 1] == '"')
    {
        str[len - 1] = '\0';
        return str + 1;
    }

    return str;
}

char *concatInput(int argc, char **input, const char *format, ...)
{
    
    char **inputSemAspas = malloc((argc - 1) * sizeof(char *));
    for (int i = 1; i < argc; i++)
    {
        inputSemAspas[i - 1] = removeAspas(input[i]);
    }

    char *ret = malloc(256 * sizeof(char));

    // Prepara os argumentos para passar ao vsprintf
    va_list parametros;
    //Inicializa os parametros com o ultimo argumento finito
    va_start(parametros, format);
    vsprintf(ret, format, parametros);  // Formata a string de acordo com o formato fornecido
    va_end(parametros);

    free(inputSemAspas);
    return ret;
}



void writeGArrayToFIFO(GArray *array, const char *fifoPath) {
    mkfifo(fifoPath, 0666);

    int fd = open(fifoPath, O_WRONLY);
    if (fd == -1) {
        perror("Erro ao abrir FIFO");
        return;
    }

    char buffer[16384] = "[";
    for (guint i = 0; i < array->len; i++) {
        if(g_array_index(array, int, i) == 0 && array->len == 1)break;
        char temp[32] = "";
        snprintf(temp, sizeof(temp), "%d", g_array_index(array, int, i));
        strcat(buffer, temp);
        if (i < array->len - 1) strcat(buffer, ",");
    }
    strcat(buffer, "]\n");

    write(fd, buffer, strlen(buffer));
    
    close(fd);
}






void printGArray(GArray *array)
{
    printf("[");

    for (guint i = 0; i < array->len; i++)
    {
        int item = g_array_index(array, int, i);
        printf("%d,",item);
    }
    printf("]\n");

}





int isDigitsOnly(char* str) {
    for (int i = 0; str[i]; i++) {
        if (!isdigit(str[i])) return 0;
    }
    return 1;
}


int validaInput(char** argv) {
    int titleSize = strlen(argv[2]);
    int authorsSize = strlen(argv[3]); 
    int yearSize = strlen(argv[4]);
    int pathSize = strlen(argv[5]);

    if (titleSize > 250) {
        printf("Erro: O título é demasiado longo (máximo 200 caracteres).\n");
        return 0;
    }
    if (authorsSize > 100) {
        printf("Erro: Os autores têm demasiados caracteres (máximo 200 caracteres).\n");
        return 0;
    }
    if (yearSize > 4) {
        printf("Erro: O ano deve ter no máximo 4 dígitos.\n");
        return 0;
    }
    if (pathSize > 100) {
        printf("Erro: O caminho é demasiado longo (máximo 64 caracteres).\n");
        return 0;
    }

    return 1;
}










