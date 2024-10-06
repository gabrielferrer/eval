#ifndef EVAL_H
#define EVAL_H

#include <stdbool.h>
#include "poker.h"

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
	double winProbability;
	double loseProbability;
	double tieProbability;
} equity_t;

typedef struct
{
	rules_t rules;                          // Poker rules used for evaluation.
	int nPlayers;                           // How many players.
	int nBoardCards;                        // How many cards over the board.
	int nHoleCards;                         // How many hole cards per player.
	int nDeadCards;                         // How many dead cards.
	card_t* boardCards;                     // Cards over the board.
	card_t* holeCards[MAX_PLAYERS];         // Hole cards by player.
	card_t* deadCards;                      // Dead cards.
	int nCores;                             // How many CPUs in the system.
	equity_t equities[MAX_PLAYERS];         // Returned equities by player.
	int nBoards;                            // Returned total boards evaluated.
	int errors;                             // Returned error flags.
} eval_t;

typedef struct { rank_t rank; int count; } group_t;

typedef struct
{
	group_t groups[MAX_GROUPS];             // Groups of cards.
	int nGroups;                            // How many groups of cards.
	int nConsecutiveRanks;                  // How many cards with consecutive ranks.
	int nSameSuit;                           // How many cards with same suit.
	board_t orderedCards;                   // Ordered input cards.
	hand_rank_t handRank;                   // Hand rank found.
	group_t* lowPair;                       // Pointer into groups to low pair if hand rank is PAIR/TWO_PAIR/FULL_HOUSE.
	group_t* highPair;                      // Pointer into groups to high pair if hand rank is TWO_PAIR.
	group_t* trips;                         // Pointer into groups to set of 3 cards if hand rank is THREE_OF_A_KIND/FULL_HOUSE.
	group_t* four;                          // Pointer into groups to set of 4 cards if hand rank is FOUR_OF_A_KIND.
	card_t* singleGroupCards[BOARD_SIZE];   // Pointers to cards into ordered_cards that are single group.
	int nSingleGroupCards;                  // How many cards with single group.
} hand_rank_result_t;

void HandRank (board_t board, hand_rank_result_t* result);

int Compare (board_t board1, board_t board2);

bool Eval (eval_t* evalData);

#endif
