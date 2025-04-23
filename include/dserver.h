#ifndef DSERVER_H
#define DSERVER_H

typedef struct index Index; 

char* getPath(Index* indice);
int getPidCliente(Index* indice);
int getStructSize();
Index* getDeletedIndex();
int getOrder(Index* indice);
#endif
