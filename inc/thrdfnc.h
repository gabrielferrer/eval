#ifndef THRDFNC_H
#define THRDFNC_H

#include "poker.h"
#include "thread.h"

#define MAX_GROUPS 5

struct group_t { enum rank_t rank; int count; };

struct hand_rank_result_t
{
	struct group_t groups[MAX_GROUPS];           // Groups of cards.
	int nGroups;                                 // How many groups of cards.
	int nConsecutiveRanks;                       // How many cards with consecutive ranks.
	int nSameSuit;                               // How many cards with same suit.
	struct card_t orderedCards[BOARD_SIZE];      // Ordered input cards.
	enum hand_rank_t handRank;                   // Hand rank found.
	struct group_t* lowPair;                     // Pointer into groups to low pair if hand rank is PAIR/TWO_PAIR/FULL_HOUSE.
	struct group_t* highPair;                    // Pointer into groups to high pair if hand rank is TWO_PAIR.
	struct group_t* trips;                       // Pointer into groups to set of 3 cards if hand rank is THREE_OF_A_KIND/FULL_HOUSE.
	struct group_t* four;                        // Pointer into groups to set of 4 cards if hand rank is FOUR_OF_A_KIND.
	struct card_t* singleGroupCards[BOARD_SIZE]; // Pointers to cards into ordered_cards that are single group.
	int nSingleGroupCards;                       // How many cards with single group.
};

struct thread_result_t
{
	int wins;
	int ties;
};

struct thread_args_t
{
#ifdef DEBUG
	int threadNr;
#endif
	int nPlayers;                                    // How many players.
	int nBoardCards;                                 // How many cards over the board.
	int nHoleCards;                                  // How many hole cards per player.
	int nCards;                                      // How many cards there are into deck.
	int nCombinations;                               // How many combinations a thread must process.
	int nCombinationCards;                           // How many cards per combination.
	enum rules_t rules;                              // Poker rules used for evaluation.
	struct card_t boardCards[BOARD_SIZE];            // Cards over the board.
	struct card_t holeCards[MAX_PLAYERS][MAX_CARDS]; // Hole cards by player.
	struct card_t cards[DECK_SIZE];                  // Cards left on deck.
	int indexes[BOARD_SIZE];                         // Initialized indexes to cards. Used to build combinations.
	struct thread_result_t results[MAX_PLAYERS];     // Results of calculations done by thread.
};

void HandRank (struct card_t board[BOARD_SIZE], struct hand_rank_result_t* result);

int Compare (struct card_t board1[BOARD_SIZE], struct card_t board2[BOARD_SIZE]);

THREAD_FUNC_RET_TYPE ThreadFunction (void* args);

#endif
