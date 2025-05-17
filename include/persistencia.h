#ifndef PERSISTENCIA_H
#define PERSISTENCIA_H
#include "index.h"
#include "lruCache.h"
#include <glib.h>

void writeDisco(Index* indice);


Index* searchDisco(int ordem);

int removeDisco(int ordem);


void cachePopulateDisco(int cacheSize,LRUCache* cache);

GArray* getIndexsFromCacheAndDisc(LRUCache* cache);
#endif 
