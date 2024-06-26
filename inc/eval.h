#ifndef EVAL_H
#define EVAL_H

#include <stdbool.h>
#include "poker.h"

#define DECK_SIZE 52
#define MAX_PLAYERS 10
#define MAX_GROUPS 5

// Helper macros.

#define INDEX(rank, suit) (rank - 2 << 2) + suit - 1

// Eval error flags.

#define DUPLICATED_CARD_FLAG 1
#define INSUFFICIENT_COMBINATION_CARDS 2
#define INVALID_BOARD_CARDS_COUNT 4
#define INVALID_POKER_RULES 8

typedef struct
{
	int wins;
	int ties;
	double win_probability;
	double lose_probability;
	double tie_probability;
} equity_t;

typedef struct
{
	rules_t rules;
	int players;
	int board_cards_count;
	int hole_cards_count;
	int dead_cards_count;
	card_t* board_cards;
	card_t* hole_cards[MAX_PLAYERS];
	card_t* dead_cards;
	equity_t equities[MAX_PLAYERS];
	int total_boards;
	int errors;
} eval_t;

typedef struct { rank_t rank; int count; } group_t;

typedef struct
{
	group_t groups[MAX_GROUPS];             // Groups of cards.
	int group_count;                        // How many groups of cards.
	int consecutive_ranks;                  // How many cards with consecutive ranks.
	int same_suit;                          // How many cards with same suit.
	board_t ordered_cards;                  // Ordered input cards.
	hand_rank_t hand_rank;                  // Hand rank found.
	group_t* low_pair;                      // Pointer into groups to low pair if hand rank is PAIR/TWO_PAIR/FULL_HOUSE.
	group_t* high_pair;                     // Pointer into groups to high pair if hand rank is TWO_PAIR.
	group_t* trips;                         // Pointer into groups to set of 3 cards if hand rank is THREE_OF_A_KIND/FULL_HOUSE.
	group_t* four;                          // Pointer into groups to set of 4 cards if hand rank is FOUR_OF_A_KIND.
	card_t* single_group_cards[BOARD_SIZE]; // Pointers to cards into ordered_cards that are single group.
	int single_group_cards_count;           // How many cards with single group.
} hand_rank_result_t;

void hand_rank(board_t board, hand_rank_result_t* result);

int compare(board_t board1, board_t board2);

bool eval(eval_t* eval_data);

#endif
