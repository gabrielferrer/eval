#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "enum.h"

bool next(int x[], int k, int c, int i)
{
	if (i < 0)
	{
		return true;
	}

	if (x[i] + 1 < c)
	{
		x[i]++;

		if (i < k-1 && x[i] + 1 < x[i+1])
		{
			for (int j = i+1; j < k; j++)
			{
				x[j] = x[j-1] + 1;
			}
		}

		return false;
	}

	return next(x, k, x[i], i-1);
}

bool adjust(int x[], int k, int c)
{
	return next(x, k, c, k - 1);
}

combination_info_t* initialize(card_t* set, int set_size, int combination_size, int buffer_size)
{
	if (set == NULL || combination_size < 0 || combination_size > set_size || buffer_size < 1)
	{
		return NULL;
	}

	combination_info_t* info = (combination_info_t*)malloc(sizeof(combination_info_t));

	if (info == NULL)
	{
		return NULL;
	}

	info->set = set;
	info->set_size = set_size;
	info->combination_size = combination_size;
	info->combination_bytes = (combination_size == 0 ? set_size : info->combination_size) * sizeof(card_t);
	info->indexes = combination_size == 0 ? NULL : (int*)malloc(info->combination_size * sizeof(int));
	info->combination_buffer = (card_t*)malloc((combination_size == 0 ? set_size : combination_size * buffer_size) * sizeof(card_t));
	info->buffer_size = combination_size == 0 ? 1 : buffer_size;
	info->current_combination = combination_size == 0 ? NULL : (card_t*)malloc(combination_size * sizeof(card_t));

	// Initialize indexes to point to first elements.
	for (int i = 0; i < info->combination_size; i++)
	{
		info->indexes[i] = i;
	}

	return info;
}

void dispose(combination_info_t* info)
{
	if (info == NULL)
	{
		return;
	}

	if (info->current_combination != NULL)
	{
		free(info->current_combination);
	}

	if (info->combination_buffer != NULL)
	{
		free(info->combination_buffer);
	}

	if (info->indexes != NULL)
	{
		free(info->indexes);
	}

	free(info);
}

bool combinations(combination_info_t* info)
{
	bool done;
	card_t* buffer_offset = info->combination_buffer;
	info->combination_count = 0;

	if (info->combination_size == 0)
	{
		memcpy(buffer_offset, info->set, info->combination_bytes);
		info->combination_count++;
		return false;
	}

	do
	{
		// Prepare current combination.
		for (int i = 0; i < info->combination_size; i++)
		{
			info->current_combination[i].rank = info->set[info->indexes[i]].rank;
			info->current_combination[i].suit = info->set[info->indexes[i]].suit;
		}

		// Copy combination to buffer.
		memcpy(buffer_offset, info->current_combination, info->combination_bytes);
		// Adjust destination buffer index.
		buffer_offset += info->combination_size;
		info->combination_count++;
		// Adjust combination indexes for next combination.
		done = adjust(info->indexes, info->combination_size, info->set_size);
	}
	while (!done && info->combination_count < info->buffer_size);

	return !done;
}
