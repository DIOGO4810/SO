#ifndef LRUCACHE_H
#define LRUCACHE_H
#include "dserver.h"

typedef struct lrucache LRUCache;

LRUCache* lruCacheNew(int totalCapacity);

void lruCacheFree(LRUCache* cache);

void lruCachePut(LRUCache* cache, int key, Index* indice);

Index* lruCacheGet(LRUCache* cache, int key);

void lruCacheRemove(LRUCache* cache, int key);

GArray* lruCacheFill(LRUCache* cache);

int lruCacheContains (LRUCache* cache,Index* indice);



#endif