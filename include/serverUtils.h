#ifndef SERVERUTILS_H
#define SERVERUTILS_H

#include <glib.h>

void findIndexsMatchParallel(char* datasetDirectory, GArray *ret, char *match, GArray *indexArray, int numProcesses);

void findIndexsMatch(char* datasetDirectory,GArray* ret, char* match,GArray* indexArray);


void respondMessageAdiciona(char *diretoria, int indice);

void respondMessageConsulta(char *diretoria, Index *indice);

void respondMessageRemove(char *diretoria, int indice);

void respondErrorMessage(char *diretoria);

int checkAsync(char *input);




void setupFIFOsAndDescriptors(int *fdRDdummyCache, int *fdRDdummyZombies);

void cleanExit(int fdOrdem, int fd, int fdRDdummyCache, int fdRDdummyZombies, LRUCache *cacheLRU);

void writeZombiePID();

void writeCacheUpdate(Index* updateCache);

void handleFIFOUpdates(int fdRDdummyZombies, int fdRDdummyCache, LRUCache *cacheLRU, int *status) ;




void printGArrayIndex(GArray *array);

#endif