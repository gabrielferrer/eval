// #include <stdlib.h>
// #include <string.h>
// #include <stdbool.h>
// #include "cmbntn.h"
// #include "enum.h"

// combination_info_t* E_Initialize (card_t* set, int setSize, int combinationSize, int bufferSize)
// {
	// if (set == NULL || combinationSize < 0 || combinationSize > setSize || bufferSize < 1)
	// {
		// return NULL;
	// }

	// combination_info_t* info = (combination_info_t*) malloc (sizeof (combination_info_t));

	// if (info == NULL)
	// {
		// return NULL;
	// }

	// info->set = set;
	// info->setSize = setSize;
	// info->combinationSize = combinationSize;
	// info->combinationBytes = (combinationSize == 0 ? setSize : combinationSize) * sizeof (card_t);
	// info->indexes = combinationSize == 0 ? NULL : (int*) malloc (combinationSize * sizeof (int));
	// info->combinationBuffer = (card_t*) malloc ((combinationSize == 0 ? setSize : combinationSize * bufferSize) * sizeof (card_t));
	// info->bufferSize = combinationSize == 0 ? 1 : bufferSize;
	// info->currentCombination = combinationSize == 0 ? NULL : (card_t*) malloc (combinationSize * sizeof (card_t));

	// // Initialize indexes to point to first elements.
	// for (int i = 0; i < combinationSize; i++)
	// {
		// info->indexes[i] = i;
	// }

	// return info;
// }

// void E_Dispose (combination_info_t* info)
// {
	// if (info == NULL)
	// {
		// return;
	// }

	// if (info->currentCombination != NULL)
	// {
		// free (info->currentCombination);
	// }

	// if (info->combinationBuffer != NULL)
	// {
		// free (info->combinationBuffer);
	// }

	// if (info->indexes != NULL)
	// {
		// free (info->indexes);
	// }

	// free (info);
// }

// bool E_Combinations (combination_info_t* info)
// {
	// bool done;
	// card_t* bufferOffset = info->combinationBuffer;
	// info->nCombinations = 0;

	// if (info->combinationSize == 0)
	// {
		// memcpy (bufferOffset, info->set, info->combinationBytes);
		// info->nCombinations++;
		// return false;
	// }

	// do
	// {
		// // Prepare current combination.
		// for (int i = 0; i < info->combinationSize; i++)
		// {
			// info->currentCombination[i].rank = info->set[info->indexes[i]].rank;
			// info->currentCombination[i].suit = info->set[info->indexes[i]].suit;
		// }

		// // Copy combination to buffer.
		// memcpy (bufferOffset, info->currentCombination, info->combinationBytes);
		// // Adjust destination buffer index.
		// bufferOffset += info->combinationSize;
		// info->nCombinations++;
		// // Adjust combination indexes for next combination.
		// done = CMB_Next (info->indexes, info->combinationSize, info->setSize);
	// }
	// while (!done && info->nCombinations < info->bufferSize);

	// return !done;
// }
