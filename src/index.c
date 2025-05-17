
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <glib.h>
#include <index.h>

struct index
{
    char title[250];
    char author[100];
    int year;
    char path[100];
    int pidZombie;
    int order;
    char messageType;
    int key;
    char keyWord[30];
    int pidCliente;
    int numProcessos;
};





// Setters
void setTitle(Index *idx, char *title)
{
    strncpy(idx->title, title, sizeof(idx->title) - 1);
    idx->title[sizeof(idx->title) - 1] = '\0';
}


void setPidCliente(Index* index, int pidCliente) {
    index->pidCliente = pidCliente;
}


void setAuthor(Index *idx, char *author)
{
    strncpy(idx->author, author, sizeof(idx->author) - 1);
    idx->author[sizeof(idx->author) - 1] = '\0';
}

void setYear(Index *idx, int year)
{
    idx->year = year;
}

void setPath(Index *idx, char *path)
{
    strncpy(idx->path, path, sizeof(idx->path) - 1);
    idx->path[sizeof(idx->path) - 1] = '\0';
}

void setPidZombie(Index *idx, int pidZombie)
{
    idx->pidZombie = pidZombie;
}

void setOrder(Index *idx, int order)
{
    idx->order = order;
}

void setMessageType(Index *idx, char messageType)
{
    idx->messageType = messageType;
}

void setKey(Index *idx, int key)
{
    idx->key = key;
}

void setKeyWord(Index *idx, char *keyWord)
{
    strncpy(idx->keyWord, keyWord, sizeof(idx->keyWord) - 1);
    idx->keyWord[sizeof(idx->keyWord) - 1] = '\0';
}

void setNumProcessos (Index *idx,int numProcessos){
    idx->numProcessos = numProcessos;
}

// Getters
char *getTitle(Index *idx)
{
    return idx->title;
}

int getNumProcessos(Index* index){
    return index->numProcessos;
}

int getPidCliente(Index* index) {
    return index->pidCliente;
}

int getStructSize(){
    return sizeof(Index);
}

char *getAuthor(Index *idx)
{
    return idx->author;
}

int getYear(Index *idx)
{
    return idx->year;
}

char *getPath(Index *idx)
{
    return idx->path;
}

int getPidZombie(Index *idx)
{
    return idx->pidZombie;
}

int getOrder(Index *idx)
{
    return idx->order;
}

char getMessageType(Index *idx)
{
    return idx->messageType;
}

int getKey(Index *idx)
{
    return idx->key;
}

char *getKeyWord(Index *idx)
{
    return idx->keyWord;
}

Index *getDeletedIndex()
{
    Index *deleted = malloc(sizeof(Index));
    if (!deleted)
        return NULL;

    deleted->title[0] = '\0';
    deleted->author[0] = '\0';
    deleted->path[0] = '\0';
    deleted->keyWord[0] = '\0';

    deleted->year = 0;
    deleted->order = -1;
    deleted->pidZombie = -1;
    deleted->messageType = '\0';
    deleted->key = -1;
    deleted->numProcessos = -1;

    return deleted;
}

void freeEstrutura(GArray *estrutura)
{
    for (guint i = 0; i < estrutura->len; i++)
    {
        if (g_array_index(estrutura, Index *, i) == NULL)
            continue;
        Index *item = g_array_index(estrutura, Index *, i);
        free(item);
    }
    g_array_free(estrutura, TRUE);
}

void printIndice(Index *indice)
{
    if (indice == NULL)
    {
        printf("Indice é Nulo\n");
        return;
    }

    printf("Title: %s | Author: %s | Year: %d | Path: %s | PidZombie: %d | Order: %d | MsgType: %c | Key: %d | Keyword: %s\n",
           indice->title,
           indice->author,
           indice->year,
           indice->path,
           indice->pidZombie,
           indice->order,
           indice->messageType,
           indice->key,
           indice->keyWord);
}
