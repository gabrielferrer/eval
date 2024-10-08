#ifndef EVAL_H
#define EVAL_H

#include <stdbool.h>
#include "poker.h"

// Helper macros.

#define INDEX(rank, suit) (rank - 2 << 2) + suit - 1

// Eval error flags.

#define DUPLICATED_CARD_FLAG 1
#define INSUFFICIENT_COMBINATION_CARDS 2
#define INVALID_BOARD_CARDS_COUNT 4
#define INVALID_POKER_RULES 8
#define INTERNAL_ERROR 16

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

bool Eval (eval_t* evalData);

#endif
