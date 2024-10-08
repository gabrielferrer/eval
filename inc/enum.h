//#ifndef ENUM_H
//#define ENUM_H

//#include <stdbool.h>
//#include "poker.h"

// typedef struct
// {
	// card_t* set;                 // Set of cards to get combinations from.
	// int setSize;                 // How many cards there are in set.
	// int combinationSize;         // How many cards per combination.
	// int combinationBytes;        // How many bytes per combination.
	// int* indexes;                // Indexes to cards into set. Used to build combinations.
	// card_t* combinationBuffer;   // Buffer for combinations to be put into.
	// int bufferSize;              // Buffer size in combinations.
	// int nCombinations;           // How many combinations there are into buffer.
	// card_t* currentCombination;  // Current generated combination.
// } combination_info_t;

/*
	Initializes a structure for use in combinations generation process.

	[Params]

		set:              a set of cards to get combinations from.
		set_size:         how many cards there are in set.
		combination_size: How many cards there are per combination.
		buffer_size:      How many combinations to generate per call.

	[Returns]

		An initialized structure or NULL if there was an error.
*/
//combination_info_t* E_Initialize (card_t* set, int setSize, int combinationSize, int bufferSize);

/*
	Disposes a previously initialized structure.
*/
//void E_Dispose (combination_info_t* info);

/*
	Calculates all the possible combinations for a given set of cards.

	[Params]

		info: an initialized structure for combination generation process.

	[Returns]

		True if there are more combinations to generate, false when process is done.
*/
//bool E_Combinations (combination_info_t* info);

//#endif
