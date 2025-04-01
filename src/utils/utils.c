#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

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
