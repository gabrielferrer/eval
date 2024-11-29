#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "status.h"

#define BUFFER_SIZE 256

void PrintWithStatus (char* line, bool passed, int length)
{
	char format[BUFFER_SIZE];

	strcpy (format, "%s");

	if (passed)
	{
		strcat (format, "\e[0;32m");
	}
	else
	{
		strcat (format, "\e[0;31m");
	}

	strcat (format, "%*s\e[0m\n");
	printf (format, line, length - strlen (line), passed ? PASSED_MSG : FAILED_MSG);
}
