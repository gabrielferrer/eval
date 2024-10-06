#include "cmbntn.h"

bool NextInternal (int x[], int k, int c, int i)
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

	return NextInternal (x, k, x[i], i-1);
}

bool Next (int x[], int k, int c)
{
	return NextInternal (x, k, c, k - 1);
}
