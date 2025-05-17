#ifndef PERSISTENCIA_H
#define PERSISTENCIA_H

#include "index.h"
#include "lruCache.h"
#include <glib.h>

/**
 * @brief Escreve uma estrutura Index no disco.
 * 
 * @param indice Ponteiro para a estrutura Index a ser guardada em disco.
 */
void writeDisco(Index* indice);

/**
 * @brief Procura um índice no disco com base na ordem fornecida.
 * 
 * @param ordem Identificador (ordem) do índice a procurar.
 * @return Ponteiro para a estrutura Index se encontrada, NULL caso contrário.
 */
Index* searchDisco(int ordem);

/**
 * @brief Remove um índice do disco com base na ordem fornecida.
 * 
 * @param ordem Identificador (ordem) do índice a remover.
 * @return 1 se a remoção foi bem-sucedida, 0 caso contrário.
 */
int removeDisco(int ordem);

/**
 * @brief Preenche a cache LRU com dados armazenados no disco.
 * 
 * @param cacheSize Número máximo de elementos a carregar para a cache.
 * @param cache Ponteiro para a estrutura da cache LRU.
 */
void cachePopulateDisco(int cacheSize, LRUCache* cache);

/**
 * @brief Obtém todos os índices disponíveis na cache e no disco.
 *        Evita duplicações, dando prioridade aos da cache.
 * 
 * @param cache Ponteiro para a cache LRU.
 * @return GArray contendo ponteiros para estruturas Index.
 */
GArray* getIndexsFromCacheAndDisc(LRUCache* cache);

#endif
