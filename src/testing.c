#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "testing.h"
#include "status.h"

#define SCREEN_COLUMNS 120

void Print (bool passed, char* format, va_list valist)
{
	char buffer[SCREEN_COLUMNS];

	memset (buffer, 0, SCREEN_COLUMNS);

    int written = vsnprintf (buffer, SCREEN_COLUMNS, format, valist);

	if (written < 0 || written >= SCREEN_COLUMNS)
	{
		return;
	}

	PrintWithStatus (buffer, passed, SCREEN_COLUMNS);
}

void PrintInt (int expected, int actual, char* format, ...)
{
	va_list valist;

	va_start (valist, format);
	Print (expected == actual, format, valist);
	va_end (valist);
}

void PrintFloat (float expected, float actual, char* format, ...)
{
	va_list valist;

	va_start (valist, format);
	Print (expected == actual, format, valist);
	va_end (valist);
}

void PrintDouble (double expected, double actual, char* format, ...)
{
	va_list valist;

	va_start (valist, format);
	Print (expected == actual, format, valist);
	va_end (valist);
}

void PrintInts (int expected[], int actual[], int length, char* format, ...)
{
	va_list valist;
	bool passed = true;

	for (int i = 0; i < length; i++)
	{
		if (expected[i] != actual[i])
		{
			passed = false;
			break;
		}
	}

	va_start (valist, format);
	Print (passed, format, valist);
	va_end (valist);
}

void PrintFloats (float expected[], float actual[], int length, char* format, ...)
{
	va_list valist;
	bool passed = true;

	for (int i = 0; i < length; i++)
	{
		if (expected[i] != actual[i])
		{
			passed = false;
			break;
		}
	}

	va_start (valist, format);
	Print (passed, format, valist);
	va_end (valist);
}

void PrintDoubles (double expected[], double actual[], int length, char* format, ...)
{
	va_list valist;
	bool passed = true;

	for (int i = 0; i < length; i++)
	{
		if (expected[i] != actual[i])
		{
			passed = false;
			break;
		}
	}

	va_start (valist, format);
	Print (passed, format, valist);
	va_end (valist);
}

void PrintAny (void* expected, void* actual, bool compare (void*, void*), char* format, ...)
{
	va_list valist;

	va_start (valist, format);
	Print (compare (expected, actual), format, valist);
	va_end (valist);
}
