#include <string.h>
#include <stdbool.h>
#include "poker.h"

bool next(int x[], int k, int c, int i)
{
	if (i < 0)
	{
		return true;
	}

	if (x[i] + 1 < c)
	{
		x[i]++;

		if (i < k && x[i] < x[i+1])
		{
			for (int j = i; j < k; j++)
			{
				x[j+1] = x[j] + 1;
			}
		}

		return false;
	}

	return next(x, k, x[i], i-1);
}

bool adjust(int x[], int k, int c)
{
	return next(x, k, c, k);
}

combination_info_t * initialize(card_t * set, int set_size, int combination_size, int buffer_size)
{
	if (set == NULL || combination_size < 0 || combination_size > set_size || buffer_size < 1)
	{
		return NULL;
	}

	combination_info_t * info = (combination_info_t *)malloc(sizeof(combination_info_t));

	if (info == NULL)
	{
		return NULL;
	}

	info->set = set;
	info->set_size = set_size;
	info->combination_size = combination_size;
	info->indexes = (int *)malloc(info->combination_size);
	info->combination_buffer = (card_t *)malloc(combination_size * sizeof(card_t) * buffer_size);
	info->current_combination = (card_t *)malloc(combination_size * sizeof(card_t));

	return info;
}

void dispose(combination_info_t * info)
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

bool combinations(combination_info_t * info)
{
	int size = info->combination_size * sizeof(card_t);

	// Initialize indexes to point to first elements.
	for (int i = 0; i < info->combination_size; i++)
	{
		info->indexes[i] = i;
	}

	bool done;
	int dp = 0;
	int info->combination_count = 0;

	do
	{
		// Prepare current combination.
		for (int i = 0; i < info->combination_size; i++)
		{
			combination[i] = info->set[info->indexes[i]];
		}

		// Copy combination to buffer.
		memcpy(info->combination_buffer[dp], combination, size);
		// Adjust destination buffer index.
		dp += size;
		info->combination_count++;
		// Adjust combination indexes for next combination.
		done = adjust(info->indexes, info->combination_size, info->set_size);
	}
	while (!done && info->combination_count < info->buffer_size);

	return !done;
}
