#include <stdio.h>
#include "debug.h"
#include "misc.h"

void D_write_side_by_side_boards(char* path, board_t best, board_t worst)
{
	char card_buffer[3];
	FILE* output = fopen(path, "a");

	if (output == NULL)
	{
		return;
	}

	for (int i = 0; i < BOARD_SIZE; i++)
	{
		card_to_string(&best[i], card_buffer);
		fwrite(card_buffer, 1, 2, output);
	}

	fwrite(" ", 1, 1, output);

	for (int i = 0; i < BOARD_SIZE; i++)
	{
		card_to_string(&worst[i], card_buffer);
		fwrite(card_buffer, 1, 2, output);
	}

	fwrite("\n", 1, 1, output);

	fclose(output);
}

void D_write_boards(char* path, board_t* boards, int count)
{
	char card_buffer[3];
	FILE* output = fopen(path, "a");

	if (output == NULL)
	{
		return;
	}

	for (int i = 0; i < count; i++)
	{
		for (int j = 0; j < BOARD_SIZE; j++)
		{
			card_to_string(&boards[i][j], card_buffer);
			fwrite(card_buffer, 1, 2, output);
		}

		fwrite("\n", 1, 1, output);
	}

	fclose(output);
}
