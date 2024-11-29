#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <stdbool.h>
#include "status.h"

#define GREEN 2
#define RED 4
#define WHITE 15

void PrintWithStatus (char* line, bool passed, int length)
{
	HANDLE handle = GetStdHandle (STD_OUTPUT_HANDLE);

	if (handle == INVALID_HANDLE_VALUE)
	{
		return;
	}

	printf ("%s", line);

	if (passed)
	{
		SetConsoleTextAttribute(handle, GREEN);
	}
	else
	{
		SetConsoleTextAttribute(handle, RED);
	}

	printf ("%*s\n", length - strlen (line), passed ? PASSED_MSG : FAILED_MSG);

	SetConsoleTextAttribute(handle, WHITE);
}
