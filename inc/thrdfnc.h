#ifndef THRDFNC_H
#define THRDFNC_H

#include "poker.h"
#include "thread.h"

#define MAX_GROUPS 5

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

typedef struct
{
	int wins;
	int ties;
} thread_result_t;

typedef struct
{
	rules_t rules;                          // Poker rules used for evaluation.
	int nPlayers;                           // How many players.
	int nBoardCards;                        // How many cards over the board.
	int nHoleCards;                         // How many hole cards per player.
	card_t* boardCards;                     // Cards over the board.
	card_t* holeCards[MAX_PLAYERS];         // Hole cards by player.
	card_t* cards;                          // Cards left on deck.
	int nCards;                             // How many cards there are into deck.
	int nCombinations;                      // How many combinations a thread must process.
	int nCombinationCards;                  // How many cards per combination.
	int* indexes;                           // Initialized indexes to cards. Used to build combinations.
	thread_result_t results[MAX_PLAYERS];   // Results of calculations done by thread.
} thread_args_t;

typedef struct
{
	thread_id_t id;
	thread_args_t args;
} thread_t;

void HandRank (board_t board, hand_rank_result_t* result);

int Compare (board_t board1, board_t board2);

THREAD_FUNC_RET_TYPE ThreadFunction (void* args);

#endif
