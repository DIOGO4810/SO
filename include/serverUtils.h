#ifndef SERVERUTILS_H
#define SERVERUTILS_H

#include <glib.h>

void findIndexsMatchParallel(GArray *ret, char *match, GArray *indexArray, int numProcesses);


#endif