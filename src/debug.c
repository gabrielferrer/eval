#include <stdio.h>
#include "debug.h"
#include "misc.h"

void D_WriteSideBySideBoards (char* path, board_t best, board_t worst)
{
	char cardBuffer[3];
	FILE* output = fopen (path, "a");

	if (output == NULL)
	{
		return;
	}

	for (int i = 0; i < BOARD_SIZE; i++)
	{
		cardToString (&best[i], cardBuffer);
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

void D_WriteBoards (char* path, board_t* boards, int count)
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
