#ifndef TESTING_H
#define TESTING_H

#include <stdbool.h>

void PrintInt (int expected, int actual, char* format, ...);

void PrintFloat (float expected, float actual, char* format, ...);

void PrintDouble (double expected, double actual, char* format, ...);

void PrintInts (int expected[], int actual[], int length, char* format, ...);

void PrintFloats (float expected[], float actual[], int length, char* format, ...);

void PrintDoubles (double expected[], double actual[], int length, char* format, ...);

void PrintAny (void* expected, void* actual, bool compare (void*, void*), char* format, ...);

#endif
