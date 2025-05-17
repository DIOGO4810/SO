#ifndef LRUCACHE_H
#define LRUCACHE_H

#include "index.h"
#include <glib.h>

/**
 * @brief Tipo opaco que representa uma cache LRU (Least Recently Used).
 */
typedef struct lrucache LRUCache;

/**
 * @brief Cria uma nova cache LRU com uma capacidade máxima.
 * 
 * @param totalCapacity Número máximo de elementos que a cache pode conter.
 * @return Ponteiro para a nova cache LRU.
 */
LRUCache* lruCacheNew(int totalCapacity);

/**
 * @brief Liberta a memória associada à cache LRU.
 * 
 * @param cache Ponteiro para a cache a ser libertada.
 */
void lruCacheFree(LRUCache* cache);

/**
 * @brief Insere ou atualiza um índice na cache, movendo-o para o topo da utilização.
 *        Se a cache estiver cheia, remove o elemento menos recentemente usado.
 * 
 * @param cache Ponteiro para a cache.
 * @param key Chave associada ao índice.
 * @param indice Ponteiro para a estrutura Index a armazenar.
 */
void lruCachePut(LRUCache* cache, int key, Index* indice);

/**
 * @brief Obtém o índice associado a uma chave e atualiza a sua posição como mais recentemente usado.
 * 
 * @param cache Ponteiro para a cache.
 * @param key Chave do índice a obter.
 * @return Ponteiro para o índice, ou NULL se a chave não existir.
 */
Index* lruCacheGet(LRUCache* cache, int key);

/**
 * @brief Remove um elemento da cache com base na chave.
 * 
 * @param cache Ponteiro para a cache.
 * @param key Chave do elemento a remover.
 */
void lruCacheRemove(LRUCache* cache, int key);

/**
 * @brief Retorna um GArray com os elementos da cache por ordem de utilização.
 * 
 * @param cache Ponteiro para a cache.
 * @return GArray contendo os elementos da cache.
 */
GArray* lruCacheFill(LRUCache* cache);

/**
 * @brief Verifica se uma determinada estrutura Index está presente na cache.
 * 
 * @param cache Ponteiro para a cache.
 * @param indice Ponteiro para a estrutura Index a verificar.
 * @return 1 se estiver presente, 0 caso contrário.
 */
int lruCacheContains(LRUCache* cache, Index* indice);

/**
 * @brief Imprime o conteúdo da cache para debug ou visualização.
 * 
 * @param cache Ponteiro para a cache.
 */
void lruCachePrint(LRUCache* cache);

#endif
