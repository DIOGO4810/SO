#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <glib.h>
#include "dserver.h"
#include "lruCache.h"
#include "utils.h"


typedef struct Node {
    int key;
    Index* indice;
    struct Node* prev;
    struct Node* next;
} Node;

struct lrucache {
    int totalCapacity;
    int currentSize;
    GHashTable* table;  
    Node* head;       
    Node* tail;        
};

void nodeFree(gpointer data) {
    Node* node = (Node*)data;
    free(node->indice);
    free(node);
}


LRUCache* lruCacheNew(int totalCapacity) {
    LRUCache* cache = malloc(sizeof(LRUCache));
    cache->totalCapacity = totalCapacity;
    cache->currentSize = 0;
    cache->head = NULL;
    cache->tail = NULL;

    cache->table = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, (GDestroyNotify)nodeFree);
    return cache;
}


void lruCacheFree(LRUCache* cache) {
    g_hash_table_destroy(cache->table);
    free(cache);
}
void removeNode(LRUCache* cache, Node* node) {
    // Se o nó tiver um anterior (não é a cabeça), liga o anterior ao próximo
    if (node->prev) 
        node->prev->next = node->next;
    else 
        // Se não tiver anterior, então era a cabeça — atualiza a cabeça para o próximo
        cache->head = node->next;

    // Se o nó tiver um próximo (não é a cauda), liga o próximo ao anterior
    if (node->next) 
        node->next->prev = node->prev;
    else 
        // Se não tiver próximo, então era a cauda — atualiza a cauda para o anterior
        cache->tail = node->prev;
}

// Remove o nó menos recentemente usado (no fim)
Node* removeTail(LRUCache* cache) {
    Node* tail = cache->tail;
    if (tail)
        removeNode(cache, tail);
    return tail;
}


void insertToHead(LRUCache* cache, Node* node) {
    // O novo nó será o primeiro, então não tem anterior
    node->prev = NULL;

    // O próximo do novo nó é o nó que atualmente está na cabeça
    node->next = cache->head;

    // Se já existia um nó na cabeça, atualiza o seu prev para apontar para o novo nó
    if (cache->head) 
        cache->head->prev = node;

    // Atualiza a cabeça da lista para o novo nó
    cache->head = node;

    // Se a lista estava vazia (tail == NULL), a cauda também deve apontar para o novo nó
    if (cache->tail == NULL) 
        cache->tail = node;
}


void moveToHead(LRUCache* cache, Node* node) {
    removeNode(cache, node);
    insertToHead(cache, node);
}



void lruCachePut(LRUCache* cache, int key, Index* indice) {
    Node* node = g_hash_table_lookup(cache->table, GINT_TO_POINTER(key));

    if (node) {
        node->indice = indice;
        moveToHead(cache, node);
        return;
    }

    // Novo nó
    Node* newNode = malloc(sizeof(Node));
    newNode->key = key;
    newNode->indice = indice;
    newNode->prev = newNode->next = NULL;
    insertToHead(cache, newNode);

    g_hash_table_insert(cache->table, GINT_TO_POINTER(key), newNode);
    cache->currentSize++;

    // Se exceder capacidade, remove o LRU
    if (cache->currentSize > cache->totalCapacity) {
        Node* tail = removeTail(cache);
        g_hash_table_remove(cache->table, GINT_TO_POINTER(tail->key));
        cache->currentSize--;
    }
}

Index* lruCacheGet(LRUCache* cache, int key) {
    Node* node = g_hash_table_lookup(cache->table, GINT_TO_POINTER(key));
    if (!node) return NULL;
    // moveToHead(cache, node); Inutil já que as consultas apenas acontecem nos forks
    return node->indice;
}

void lruCacheRemove(LRUCache* cache, int key){
    Node* node = g_hash_table_lookup(cache->table, GINT_TO_POINTER(key));
    if (!node) return;

    removeNode(cache,node);
    g_hash_table_remove(cache->table,GINT_TO_POINTER(key));
    cache->currentSize--;

}


GArray* lruCacheFill(LRUCache* cache){
    GArray* indexArray = g_array_new(FALSE, FALSE, sizeof(Index*));

    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, cache->table);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        Node* node = (Node*)value;
        g_array_append_val(indexArray, node->indice);
    }

    return indexArray;
}

int lruCacheContains(LRUCache* cache, Index* indice) {
    void* found = g_hash_table_lookup(cache->table, GINT_TO_POINTER(getOrder(indice)));
    return found != NULL;
}






void lruCachePrint(LRUCache* cache) {
    if (cache == NULL) {
        printf("Cache não inicializada.\n");
        return;
    }

    printf("\nCache LRU: (Total Capacity: %d, Current Size: %d)\n", cache->totalCapacity, cache->currentSize);
    int i = 0;
    Node* currentNode = cache->head;
    while (i < cache->currentSize) {
        printf("Key: %d\n", currentNode->key);
        printIndice(currentNode->indice);
        currentNode = currentNode->next;
        i++;
    }
    printf("\n");
}










