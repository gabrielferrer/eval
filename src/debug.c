#include <stdio.h>
#include <stdarg.h>
#include "debug.h"
#include "misc.h"

#define PATH_SIZE 1000

void D_WriteSideBySideBoards (struct card_t best[BOARD_SIZE], struct card_t worst[BOARD_SIZE], char* format, ...)
{
	char path[PATH_SIZE];
	char cardBuffer[3];
	va_list valist;

	va_start (valist, format);
	int written = vsnprintf (path, PATH_SIZE, format, valist);
	va_end (valist);

	if (written < 0 || written >= PATH_SIZE)
	{
		return;
	}

	FILE* output = fopen (path, "a");

	if (output == NULL)
	{
		return;
	}

	for (int i = 0; i < BOARD_SIZE; i++)
	{
		CardToString (&best[i], cardBuffer);
		fwrite (cardBuffer, 1, 2, output);
	}

	fwrite (" ", 1, 1, output);

	for (int i = 0; i < BOARD_SIZE; i++)
	{
		CardToString (&worst[i], cardBuffer);
		fwrite (cardBuffer, 1, 2, output);
	}

	fwrite ("\n", 1, 1, output);

	fclose (output);
}

void D_WriteBoards (struct card_t (*boards)[BOARD_SIZE], int count, char* path)
{
	char cardBuffer[3];
	FILE* output = fopen (path, "a");

	if (output == NULL)
	{
		return;
	}

	for (int i = 0; i < count; i++)
	{
		for (int j = 0; j < BOARD_SIZE; j++)
		{
			CardToString (&boards[i][j], cardBuffer);
			fwrite (cardBuffer, 1, 2, output);
		}

		fwrite ("\n", 1, 1, output);
	}

	fclose (output);
}
