#ifndef INDEX_H
#define INDEX_H

#include<glib.h>
typedef struct index Index; 



int getStructSize();

Index* getDeletedIndex();

void freeEstrutura(GArray *estrutura);
void printIndice(Index *indice);


// Setters
void setTitle(Index *idx, char *title);
void setAuthor(Index *idx, char *author);
void setYear(Index *idx, int year);
void setPath(Index *idx, char *path);
void setPidZombie(Index *idx, int pidZombie);
void setOrder(Index *idx, int order);
void setMessageType(Index *idx, char messageType);
void setKey(Index *idx, int key);
void setKeyWord(Index *idx, char *keyWord);
void setPidCliente(Index* index, int pidCliente);
void setNumProcessos (Index *idx,int numProcessos);


// Getters
char* getTitle(Index *idx);
int getPidCliente(Index* index);
char* getAuthor(Index *idx);
int getYear(Index *idx);
char* getPath(Index *idx);
int getPidZombie(Index *idx);
int getOrder(Index *idx);
char getMessageType(Index *idx);
int getKey(Index *idx);
char* getKeyWord(Index *idx);
int getNumProcessos (Index *idx);


#endif