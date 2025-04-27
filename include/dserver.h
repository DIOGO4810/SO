#ifndef DSERVER_H
#define DSERVER_H

#include<glib.h>
typedef struct index Index; 

char* getPath(Index* indice);

char* getTitle(Index* indice);

char* getAuthor(Index* indice);

int getYear (Index* indice);

int getPidCliente(Index* indice);

int getStructSize();

Index* getDeletedIndex();

int getOrder(Index* indice);

void printIndice(Index *indice);

#endif
