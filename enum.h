#ifndef ENUM_H
#define ENUM_H

#include "poker.h"

typedef struct
{
	card_t set[];                 // Set of cards to get combinations from.
	int set_size;                 // How many cards there are in set.
	int combination_size;         // How many cards per combination.
	int * indexes;                // Indexes to cards into set. Used to build combinations.
	card_t * combination_buffer;  // Buffer for combinations to be put into.
	int buffer_size;              // Buffer size in combinations.
	int combination_count;        // How many combinations there are into buffer.
	cart_t * current_combination; // Current generated combination.
} combination_info_t;

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
combination_info_t * initialize(card_t * set, int set_size, int combination_size, int buffer_size);

/*
	Disposes a previously initialized structure.
*/
void dispose(combination_info_t * info);

/*
	Calculates all the possible combinations for a given set of cards.

	[Params]

		info: an initialized structure for combination generation process.

	[Returns]

		True if there are more combinations to generate, false when process is done.
*/
bool combinations(combination_info_t * info);

#endif
