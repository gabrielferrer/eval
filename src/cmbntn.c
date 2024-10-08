#include "cmbntn.h"

bool CMB_NextInternal (int x[], int k, int c, int i)
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

	return CMB_NextInternal (x, k, x[i], i-1);
}

bool CMB_Next (int x[], int k, int c)
{
	return CMB_NextInternal (x, k, c, k - 1);
}

long int CMB_Product (long int from, long int to)
{
	long int r = 1;

	for (long int i = from; i <= to; i++)
	{
		r *= i;
	}

	return r;
}

long int CMB_Combination (long int n, long int k)
{
	if (n < k)
	{
		return 0;
	}

	if (n == k)
	{
		return 1;
	}

	return k > n - k ? CMB_Product (k + 1, n) / CMB_Product (1, n - k) : CMB_Product (n - k + 1, n) / CMB_Product (1, k);
}
