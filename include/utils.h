#ifndef UTILS_H
#define UTILS_H

#include <glib.h>

/**
 * @brief Concatena múltiplas strings a partir do array de entrada, aplicando um formato.
 * 
 * @param argc Número de strings no array input.
 * @param input Array de strings a concatenar.
 * @param format String de formato (similar a printf) para formatar a concatenação.
 * @param ... Argumentos adicionais para o formato.
 * @return Ponteiro para a string concatenada alocada dinamicamente. Deve ser libertada pelo utilizador.
 */
char *concatInput(int argc, char **input, const char *format, ...);

/**
 * @brief Escreve o conteúdo de um GArray num FIFO (named pipe).
 * 
 * @param array GArray contendo os dados a escrever.
 * @param fifoPath Caminho para o FIFO onde os dados serão escritos.
 */
void writeGArrayToFIFO(GArray *array, const char *fifoPath);

/**
 * @brief Imprime o conteúdo de um GArray para o stdout.
 * 
 * @param array GArray a imprimir.
 */
void printGArray(GArray *array);

/**
 * @brief Valida os argumentos de entrada (argv).
 * 
 * @param argv Array de strings com os argumentos.
 * @return 1 se os argumentos forem válidos, 0 caso contrário.
 */
int validaInput(char** argv);

#endif
