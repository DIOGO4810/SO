#ifndef SERVERUTILS_H
#define SERVERUTILS_H

#include <glib.h>

/**
 * @brief Procura em paralelo índices que correspondam a uma string de pesquisa.
 * 
 * @param datasetDirectory Diretório onde os dados estão armazenados.
 * @param ret GArray onde serão armazenados os índices encontrados.
 * @param match String de correspondência para pesquisa.
 * @param indexArray Array com índices para pesquisar.
 * @param numProcesses Número de processos paralelos a usar.
 */
void findIndexsMatchParallel(char* datasetDirectory, GArray *ret, char *match, GArray *indexArray, int numProcesses);

/**
 * @brief Procura índices que correspondam a uma string de pesquisa (sequencial).
 * 
 * @param datasetDirectory Diretório onde os dados estão armazenados.
 * @param ret GArray onde serão armazenados os índices encontrados.
 * @param match String de correspondência para pesquisa.
 * @param indexArray Array com índices para pesquisar.
 */
void findIndexsMatch(char* datasetDirectory, GArray* ret, char* match, GArray* indexArray);

/**
 * @brief Envia uma mensagem de resposta para adicionar um índice.
 * 
 * @param diretoria Diretório ou caminho relacionado.
 * @param indice Índice a adicionar.
 */
void respondMessageAdiciona(char *diretoria, int indice);

/**
 * @brief Envia uma mensagem de resposta com um índice consultado.
 * 
 * @param diretoria Diretório ou caminho relacionado.
 * @param indice Ponteiro para o índice consultado.
 */
void respondMessageConsulta(char *diretoria, Index *indice);

/**
 * @brief Envia uma mensagem de resposta para remover um índice.
 * 
 * @param diretoria Diretório ou caminho relacionado.
 * @param indice Índice a remover.
 */
void respondMessageRemove(char *diretoria, int indice);

/**
 * @brief Envia uma mensagem de erro.
 * 
 * @param diretoria Diretório ou caminho relacionado.
 */
void respondErrorMessage(char *diretoria);

/**
 * @brief Verifica se o tipo da mensagem indica operação assíncrona.
 * 
 * @param type Carácter indicando o tipo da mensagem.
 * @return 1 se for assíncrona, 0 caso contrário.
 */
int checkAsync(char type);


/**
 * @brief Realiza limpeza e saída do servidor, fechando descritores e libertando a cache.
 * 
 * @param fdOrdem Descritor de ficheiro para ordem.
 * @param fd Descritor geral a fechar.
 * @param cacheLRU Ponteiro para a cache LRU a libertar.
 */
void cleanExit(int fdOrdem, int fd, LRUCache *cacheLRU);

/**
 * @brief Escreve atualização enviada por um processo filho (son).
 * 
 * @param sonUpdate Ponteiro para a estrutura Index com a atualização.
 */
void writeSonUpdate(Index* sonUpdate);

/**
 * @brief Processa atualizações recebidas via FIFO e atualiza a cache LRU.
 * 
 * @param cacheLRU Ponteiro para a cache LRU.
 * @param status Ponteiro para estado (status) do processo.
 * @param receivedMessage Ponteiro para a mensagem recebida.
 * @return Valor inteiro indicando sucesso ou falha da operação.
 */
int handleFIFOUpdates(LRUCache *cacheLRU, int *status, Index *receivedMessage);

/**
 * @brief Imprime o conteúdo de um GArray contendo ponteiros para estruturas Index.
 * 
 * @param array GArray a imprimir.
 */
void printGArrayIndex(GArray *array);

#endif
