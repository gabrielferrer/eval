#include <stdio.h>
#include <stdarg.h>
#include "debug.h"
#include "misc.h"
#include "thrdfnc.h"

#define PATH_SIZE 1000
#define BUFFER_SIZE 100

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

void D_WriteBoards (struct card_t (*boards)[BOARD_SIZE], int count, char* format, ...)
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

	for (int i = 0; i < count; i++)
	{
		for (int j = 0; j < BOARD_SIZE; j++)
		{
			CardToString (&boards[i][j], cardBuffer);
			fwrite (cardBuffer, 1, 2, output);
		}

		if (i + 1 < count)
		{
			fwrite ("\n", 1, 1, output);
		}
	}

	fclose (output);
}

void D_WriteThreadArguments (struct thread_args_t* threadArgs, int combinationsLeft, char* path)
{
	char buffer[BUFFER_SIZE];
	FILE* output = fopen (path, "a");

	if (output == NULL)
	{
		return;
	}

	sprintf (buffer, "Thread number:          %d\n", threadArgs->threadNr);
	fwrite (buffer, 1, strlen (buffer), output);

	sprintf (buffer, "Nr. of players:         %d\n", threadArgs->nPlayers);
	fwrite (buffer, 1, strlen (buffer), output);

	sprintf (buffer, "Nr. of board cards:     %d\n", threadArgs->nBoardCards);
	fwrite (buffer, 1, strlen (buffer), output);

	sprintf (buffer, "Nr. of hole cards:      %d\n", threadArgs->nHoleCards);
	fwrite (buffer, 1, strlen (buffer), output);

	sprintf (buffer, "Nr. of cards at deck:   %d\n", threadArgs->nCards);
	fwrite (buffer, 1, strlen (buffer), output);

	sprintf (buffer, "Combinations left:      %d\n", combinationsLeft);
	fwrite (buffer, 1, strlen (buffer), output);

	sprintf (buffer, "Nr. of combinations:    %d\n", threadArgs->nCombinations);
	fwrite (buffer, 1, strlen (buffer), output);

	sprintf (buffer, "Nr. of cards per comb.: %d\n", threadArgs->nCombinationCards);
	fwrite (buffer, 1, strlen (buffer), output);

	sprintf (buffer, "Rules:                  %s\n", RulesToString (threadArgs->rules));
	fwrite (buffer, 1, strlen (buffer), output);

	sprintf (buffer, "Indexes:                ");
	fwrite (buffer, 1, strlen (buffer), output);

	for (int i = threadArgs->nCombinationCards - 1; i >= 0; i--)
	{
		sprintf (buffer, "i%d=%d", i, threadArgs->indexes[i]);
		fwrite(buffer, 1, strlen (buffer), output);

		if (i > 0)
		{
			fwrite (", ", 1, 2, output);
		}
		else
		{
			fwrite ("\n", 1, 1, output);
		}
	}

	fwrite ("\n", 1, 1, output);
}
