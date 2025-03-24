#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

char *concatInput(int argc, char **input)
{
    char **inputSemAspas = malloc((argc - 2) * sizeof(char *));
    char *ret = malloc(256 * sizeof(char));
    for (int i = 2; i < argc; i++)
    {
        inputSemAspas[i - 2] = removeAspas(input[i]);
    }

    sprintf(ret, "%s %s %s %s", inputSemAspas[0], inputSemAspas[1], inputSemAspas[2], inputSemAspas[3]);
    return ret;
}
