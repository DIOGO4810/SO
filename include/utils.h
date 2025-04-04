#ifndef UTILS_H
#define UTILS_H
#include <glib.h>

char *concatInput(int argc, char **input, const char *format, ...);
void writeGArrayToFIFO(GArray *array, const char *fifoPath);
void printGArray(GArray *array);
#endif 
