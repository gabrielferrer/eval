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

struct equity_t
{
	int wins;
	int ties;
	double winProbability;
	double loseProbability;
	double tieProbability;
};

struct eval_t
{
	int nPlayers;                                    // How many players.
	int nBoardCards;                                 // How many cards over the board.
	int nHoleCards;                                  // How many hole cards per player.
	int nDeadCards;                                  // How many dead cards.
	int nCores;                                      // How many CPUs in the system.
	int nBoards;                                     // Returned total boards evaluated.
	enum rules_t rules;                              // Poker rules used for evaluation.
	struct card_t boardCards[BOARD_SIZE];            // Cards over the board.
	struct card_t holeCards[MAX_PLAYERS][MAX_CARDS]; // Hole cards by player.
	struct card_t deadCards[DECK_SIZE];              // Dead cards.
	struct equity_t equities[MAX_PLAYERS];           // Returned equities by player.
	int errors;                                      // Returned error flags.
};

#ifdef DEBUG
void InitialzeIndexes (int* indexes, int nIndexes, int nCombinations, int nCards);
#endif

bool Eval (struct eval_t* evalData);

#endif
