#include <stdio.h>
#include <stddef.h>
#include <string.h>

#ifdef DEBUG
#include "debug.h"
#endif

#define SET_SIZE 52
#define INDEXES 5
#define BUFFER_SIZE 50

typedef struct
{
	long int min;
	long int max;
} range_t;

range_t ranges[INDEXES] = {	{ 0, 47 }, { 1, 48 }, { 2, 49 }, { 3, 50 }, { 4, 51 } };

long int result[INDEXES][SET_SIZE];

long int product (long int from, long int to)
{
	long int r = 1;

	for (long int i = from; i <= to; i++)
	{
		r *= i;
	}

	return r;
}

long int combination(long int n, long int k)
{
	if (n < k)
	{
		return 0;
	}

	if (n == k)
	{
		return 1;
	}

	return k > n - k ? product (k + 1, n) / product (1, n - k) : product (n - k + 1, n) / product (1, k);
}

int main (int argc, char* argv[])
{
	if (argc != 2)
	{
		return -1;
	}

	char values[INDEXES][SET_SIZE][BUFFER_SIZE];
	int maxlenghts[SET_SIZE];
	int lengths[INDEXES];

	for (int i = 0; i < INDEXES; i++)
	{
		for (int j = 0; j < SET_SIZE; j++)
		{
			result[i][j] = 0;
		}
	}

	// N = C(P5,5) + C(P4,4) + C(P3,3) + C(P2,2) + C(P1,1) + 1

	for (int i = 0; i < INDEXES; i++)
	{
		for (int j = ranges[i].min; j <= ranges[i].max; j++)
		{
			result[i][j] = combination (j, i + 1);
		}
	}

	FILE* output = fopen (argv[1], "w");

	if (output == NULL)
	{
		return -1;
	}

	for (int j = 0; j < SET_SIZE; j++)
	{
		maxlenghts[j] = 0;

		for (int i = 0; i < INDEXES; i++)
		{
			memset (values[i][j], 0, BUFFER_SIZE);
			sprintf (values[i][j], "%ld", result[i][j]);
			lengths[i] = strlen(values[i][j]);
		}

		for (int i = 0; i < INDEXES; i++)
		{
			if (lengths[i] > maxlenghts[j])
			{
				maxlenghts[j] = lengths[i];
			}
		}
	}

	for (int i = 0; i < INDEXES; i++)
	{
		for (int j = 0; j < SET_SIZE; j++)
		{
			int length = strlen(values[i][j]);

			for (int k = 0; k < maxlenghts[j] - length; k++)
			{
				fwrite (" ", sizeof (char), 1, output);
			}

			fwrite (values[i][j], sizeof (char), length, output);

			if (j + 1 < SET_SIZE)
			{
				fwrite (", ", sizeof (char), 2, output);
			}
		}

		if (i + 1 < INDEXES)
		{
			fwrite ("\n", sizeof (char), 1, output);
		}
	}

	fclose (output);

	return 0;
}
