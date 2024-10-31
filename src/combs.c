#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "cmbntn.h"

#ifdef DEBUG
#include "debug.h"
#endif

#define SET_SIZE 52
#define INDEXES 5
#define BUFFER_SIZE 50

struct range_t
{
	long int min;
	long int max;
};

bool CheckArguments(int argc, char* argv[])
{
	return argc == 2 || argc == 3 && (strcmp(argv[1], "-a") == 0 || strcmp(argv[1], "-d") == 0);
}

void PrintUsage(char* programName)
{
	printf ("Use: %s [options] path\n", programName);
	printf ("\n");
	printf ("options:\n");
	printf ("\t-a: generate combinations ascending (default).\n");
	printf ("\t-d: generate combinations descending.\n");
	printf ("\n");
}

int main (int argc, char* argv[])
{
	if (!CheckArguments(argc, argv))
	{
		PrintUsage(argv[0]);
		return 0;
	}

	char values[INDEXES][SET_SIZE][BUFFER_SIZE];
	int maxlenghts[SET_SIZE];
	int lengths[INDEXES];
	struct range_t ranges[INDEXES];
	long int result[INDEXES][SET_SIZE];
	bool asc = argc != 3 || strcmp(argv[1], "-d") != 0;

	for (int i = 0; i < INDEXES; i++)
	{
		ranges[i].min = asc ? i : 0;
		ranges[i].max = asc ? SET_SIZE - 1 : SET_SIZE - i - 1;
	}

	for (int i = 0; i < INDEXES; i++)
	{
		for (int j = 0; j < SET_SIZE; j++)
		{
			result[i][j] = 0;
		}
	}

	// N = C(P5,5) + C(P4,4) + C(P3,3) + C(P2,2) + C(P1,1) + 1
	// C:  combination function
	// Pn: position of index n (starts at zero)
	// n:  index number (starts at one)

	for (int i = 0; i < INDEXES; i++)
	{
		for (int j = ranges[i].min; j <= ranges[i].max; j++)
		{
			result[i][j] = CMB_Combination (asc ? j : SET_SIZE - j - 1, i + 1);
		}
	}

	FILE* output = fopen (argc == 2 ? argv[1] : argv[2], "w");

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
			lengths[i] = strlen (values[i][j]);
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
			int length = strlen (values[i][j]);

			if (asc)
			{
				for (int k = 0; k < maxlenghts[j] - length; k++)
				{
					fwrite (" ", sizeof (char), 1, output);
				}
			}

			fwrite (values[i][j], sizeof (char), length, output);

			if (j + 1 < SET_SIZE)
			{
				fwrite (", ", sizeof (char), 2, output);
			}

			if (!asc)
			{
				for (int k = 0; k < maxlenghts[j] - length; k++)
				{
					fwrite (" ", sizeof (char), 1, output);
				}
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
