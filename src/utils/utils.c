#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <glib.h>


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

    char buffer[1024] = "[";
    char temp[32];

    for (guint i = 0; i < array->len; i++) {
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