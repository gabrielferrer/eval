#ifndef EVAL_H
#define EVAL_H

#include <stdbool.h>
#include "poker.h"

#define DECK_SIZE 52
#define COMBINATION_SIZE 5
#define MAX_PLAYERS 10
#define MAX_GROUPS 5

// Eval error flags.

#define DUPLICATED_CARD_FLAG 1
#define INSUFFICIENT_COMBINATION_CARDS 2
#define INVALID_BOARD_CARDS_COUNT 4
#define INVALID_POKER_RULES 8

typedef struct{ double win_percent; double lose_percent; double tie_percent; } equity_t;

typedef card_t combination_t[COMBINATION_SIZE];

typedef struct
{
	rules_t rules;
	int players;
	int board_cards_count;
	int hole_cards_count;
	int dead_cards_count;
	card_t * board_cards;
	card_t * hole_cards[MAX_PLAYERS];
	card_t * dead_cards;
	equity_t * equities;
	int errors;
} eval_t;

typedef struct { rank_t rank; int count; } group_t;

typedef struct
{
	group_t groups[MAX_GROUPS];                    // Groups of cards.
	int group_count;                               // How many groups of cards.
	int consecutive_ranks;                         // How many cards with consecutive ranks.
	int same_suit;                                 // How many cards with same suit.
	combination_t ordered_cards;                   // Ordered input cards.
	hand_rank_t hand_rank;                         // Hand rank found.
	group_t * low_pair;                            // Pointer into groups to low pair if hand rank is PAIR/TWO_PAIR/FULL_HOUSE.
	group_t * high_pair;                           // Pointer into groups to high pair if hand rank is TWO_PAIR.
	group_t * trips;                               // Pointer into groups to set of 3 cards if hand rank is THREE_OF_A_KIND/FULL_HOUSE.
	group_t * four;                                // Pointer into groups to set of 4 cards if hand rank is FOUR_OF_A_KIND.
	card_t * single_group_cards[COMBINATION_SIZE]; // Pointers to cards into ordered_cards that are single group.
	int single_group_cards_count;                  // How many cards with single group.
} hand_rank_result_t;

char * hand_rank_to_string(hand_rank_t hand_rank);

bool string_to_combination(char * combination_string, combination_t combination);

void hand_rank(combination_t combination, hand_rank_result_t * result);

int compare(combination_t combination1, combination_t combination2);

bool eval(eval_t * eval_data);

#endif
