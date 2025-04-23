#ifndef PERSISTENCIA_H
#define PERSISTENCIA_H
#include "dserver.h"

void writeDisco(Index* indice);


Index* searchDisco(int ordem);

int removeCsvLine(int pidCliente);

#endif 
