#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "poker.h"
#include "enum.h"
#include "eval.h"
#include "misc.h"
#include "fsm.h"
#include "core.h"
#include "thrdfnc.h"

#ifdef DEBUG
#include "debug.h"
#endif

#define BUFFER_SIZE 10000

typedef struct
{
	int wins;
	int loses;
	int ties;
	double eqWin;
	double eqLose;
	double eqTie;
} eq_t;

hand_rank_result_t r;

void HandRankTest (char* boardString, hand_rank_t hr)
{
	board_t board;
	hand_rank_result_t result;

	if (StringToCards (boardString, board) == NULL)
	{
		printf ("Invalid input: %s", boardString);
		return;
	}

	HandRank (board, &result);

	printf ("Board: %s. Expected: %s. Actual: %s\n", boardString, HandRankToString (hr), HandRankToString (result.handRank));
}

void CompareTest (char* boardString1, char* boardString2, int e)
{
	board_t board1;
	board_t board2;
	hand_rank_result_t result1;
	hand_rank_result_t result2;

	if (StringToCards (boardString1, board1) == NULL)
	{
		printf ("Invalid input: %s", boardString1);
		return;
	}

	if (StringToCards (boardString2, board2) == NULL)
	{
		printf ("Invalid input: %s", boardString2);
		return;
	}

	printf ("1st. board: %s. 2nd. board: %s. Expected: %d. Actual: %d\n", boardString1, boardString2, e, Compare (board1, board2));
}

void EvalTest (rules_t rules, int nPlayers, char* boardCards, char* deadCards, ...)
{
	va_list valist;
	eval_t evalData;
	board_t board;
	card_t dead[DECK_SIZE];
	card_t hole[MAX_PLAYERS * MAX_CARDS];
	eq_t equities[MAX_PLAYERS];
	char cardBuffer[3];

	evalData.rules = rules;
	evalData.nPlayers = nPlayers;
	evalData.boardCards = StringToCards (boardCards, board);
	evalData.deadCards = StringToCards (deadCards, dead);

	evalData.nBoardCards = boardCards != NULL ? strlen (boardCards) / 2 : 0;
	evalData.nDeadCards = deadCards != NULL ? strlen (deadCards) / 2 : 0;

	va_start (valist, deadCards);

	evalData.nHoleCards = 0;

	for (int i = 0; i < nPlayers; i++)
	{
		char* holeCards = va_arg (valist, char*);
		evalData.holeCards[i] = &hole[i * MAX_CARDS];
		StringToCards (holeCards, evalData.holeCards[i]);

		if (holeCards != NULL && evalData.nHoleCards == 0)
		{
			evalData.nHoleCards = strlen (holeCards) / 2;
		}
	}

	evalData.nCores = GetCores ();

	for (int i = 0; i < nPlayers; i++)
	{
		equities[i].wins = va_arg (valist, int);
		equities[i].loses = va_arg (valist, int);
		equities[i].ties = va_arg (valist, int);
		equities[i].eqWin = va_arg (valist, double);
		equities[i].eqLose = va_arg (valist, double);
		equities[i].eqTie = va_arg (valist, double);
	}

	va_end (valist);

	Eval (&evalData);

	if (evalData.errors != 0)
	{
		if (evalData.errors & DUPLICATED_CARD_FLAG)
		{
			printf ("Duplicated card.\n");
		}

		if (evalData.errors & INSUFFICIENT_COMBINATION_CARDS)
		{
			printf ("Insufficient combination cards.\n");
		}

		if (evalData.errors & INVALID_BOARD_CARDS_COUNT)
		{
			printf ("Invalid board cards count.\n");
		}

		if (evalData.errors & INVALID_POKER_RULES)
		{
			printf ("Invalid poker rules.\n");
		}

		return;
	}

	for (int i = 0; i < nPlayers; i++)
	{
		int loses = evalData.nBoards - evalData.equities[i].wins - evalData.equities[i].ties;

		printf ("[Expected W/L/T - Eq W/L/T] [Actual W/L/T - Eq W/L/T]: [%i/%i/%i - %2.2f/%2.2f/%2.2f] [%i/%i/%i - %2.2f/%2.2f/%2.2f]\n", \
			equities[i].wins, equities[i].loses, equities[i].ties, equities[i].eqWin, equities[i].eqLose, equities[i].eqTie, \
			evalData.equities[i].wins, loses, evalData.equities[i].ties, \
			evalData.equities[i].winProbability * 100.0d, evalData.equities[i].loseProbability * 100.0d, \
			evalData.equities[i].tieProbability * 100.0d);
	}
}

void RankTests ()
{
	HandRankTest ("9c4cKd2h6s", HIGH_CARD);
	HandRankTest ("9c4cKdKh6s", PAIR);
	HandRankTest ("Kh9c4cKd4s", TWO_PAIR);
	HandRankTest ("8h4c4hKd4s", THREE_OF_A_KIND);
	HandRankTest ("8h4c7h5d6s", STRAIGHT);
	HandRankTest ("5s2cAh4d3d", STRAIGHT);
	HandRankTest ("3hAhTh5h8h", FLUSH);
	HandRankTest ("3hTsTh3cTd", FULL_HOUSE);
	HandRankTest ("KhKsKcJcKd", FOUR_OF_A_KIND);
	HandRankTest ("7c4c3c6c5c", STRAIGHT_FLUSH);
	HandRankTest ("JdAdQdKdTd", ROYAL_FLUSH);
}

void ComparationTests ()
{
	// High card.
	CompareTest ("7dKsAc2cJs", "JcAs7dKs6h", -1);
	CompareTest ("8dKsAc2cJs", "JcAs7dKs6h", 1);
	CompareTest ("8dKsAc2cQs", "JcAs7dKs6h", 1);
	CompareTest ("7dTsAc2c9s", "JcAs8d9s6h", -1);
	CompareTest ("7dTsAc9c8s", "TcAs8d9s7h", 0);

	// Pair
	CompareTest ("AhAd5cTd6s", "AcAs7d8s3h", 1);
	CompareTest ("AhAd5cJd6s", "AcAs7dJs3h", -1);
	CompareTest ("AhAd2cJd7s", "AcAs7dJs6h", -1);
	CompareTest ("5h4dQcJdQs", "5c4sQdJsQh", 0);
	CompareTest ("Th2dTc7dAs", "2c4sQd6s2h", 1);

	// Two pair
	CompareTest ("AhAd5cTdTs", "AcAsTc8sTh", -1);
	CompareTest ("AhAd8cTdTs", "AcAsTc8sTh", 0);
	CompareTest ("2h5d2cJdJs", "Jc6s7dJh6h", -1);

	// Three of a kind
	CompareTest ("AhAd5cAsTs", "QcAsQs8sQh", 1);
	CompareTest ("6c6s7dJc6h", "Jh5d2cJdJs", -1);

	// Straight
	CompareTest ("5h9d6c8s7s", "7c3s5s6s4h", 1);
	CompareTest ("Ah5d2c4d3s", "Tc6s8c7s9h", -1);
	CompareTest ("ThJd8c9d7s", "7c8sTdJh9h", 0);
	CompareTest ("3h2d5c4dAs", "4c5s3d2hAh", 0);

	// Flush
	CompareTest ("QdAd6dTd7d", "QhAh5h7hTh", 1);
	CompareTest ("QdAd6dTd7d", "QhAh5h8hTh", -1);
	CompareTest ("QdAd6dJd7d", "QhAh5h8hTh", 1);
	CompareTest ("QdAd6dJd7d", "KhAh5h8hTh", -1);
	CompareTest ("Qd2d6dJd7d", "Kh4h5h8hTh", -1);
	CompareTest ("QdTd5d8d4d", "Qh4h5h8hTh", 0);

	// Full house
	CompareTest ("AhAd5cAs5h", "Qc5sQs5dQh", 1);
	CompareTest ("2h5c2s2c5h", "Qc5sQs5dQh", -1);

	// Four of a kind
	CompareTest ("4sKs4c4d4h", "QdQsQc5dQh", -1);
	CompareTest ("Ts3cThTdTc", "5h5c2s5s5d", 1);

	// Straight flush
	CompareTest ("5h9h6h8h7h", "7c3c5c6c4c", 1);
	CompareTest ("Ad5d2d4d3d", "Ts6s8s7s9s", -1);
	CompareTest ("ThJh8h9h7h", "7s8sTsJs9s", 0);
	CompareTest ("3c2c5c4cAc", "4d5d3d2dAd", 0);

	// Royal flush
	CompareTest ("KhJhQhThAh", "JcQcTcAcKc", 0);
}

// void FsmCombinationsLog ()
// {
	// board_t board;
	// card_t holeCards[4];

	// StringToCards ("3h5h7sAd", hole_cards);
	// StringToCards ("KsAc2s3c7d", board);

	// FSM_ResetRules (OMAHA);
	// FSM_ResetHoleCards (holeCards, 4);
	// FSM_ResetBoardCards (board);

	// while (FSM_Next (board))
	// {
		// D_WriteBoards ("C:\\Users\\Gabriel\\Desktop\\boards.txt", &board, 1);
	// }
// }

#ifdef DEBUG
void CombinationIndexesLog ()
{
	card_t deck[DECK_SIZE];

	for (rank_t r = TWO; r <= ACE; r++)
	{
		for (suit_t s = CLUBS; s <= SPADES; s++)
		{
			int i = INDEX (r, s);
			deck[i].rank = r;
			deck[i].suit = s;
		}
	}

	combination_info_t* info = E_Initialize (deck, DECK_SIZE, BOARD_SIZE, BUFFER_SIZE);
	bool more = false;

	info->indexes[0] = DECK_SIZE - 29;
	info->indexes[1] = DECK_SIZE - 25;
	info->indexes[2] = DECK_SIZE - 19;
	info->indexes[3] = DECK_SIZE - 11;
	info->indexes[4] = DECK_SIZE - 10;

	do
	{
		more = E_Combinations (info);
		D_WriteBoards("C:\\Users\\Gabriel\\Desktop\\combinations.txt", (board_t*) info->combination_buffer, info->combination_count);
	} while (more);
}
#endif

void EvalTests ()
{
	// EvalTest (HOLDEM, 2, "KsAc2s3c7d", NULL, "3h5h", "Ts2c", 1, 0, 0, 100.0d, 0.0d, 0.0d, 0, 1, 0, 0.0d, 100.0d, 0.0d);
	// EvalTest (HOLDEM, 2, "KsAc2s3c", NULL, "3h5h", "Ts2c", 39, 5, 0, 88.64d, 11.36d, 0.0d, 5, 39, 0, 11.36d, 88.64d, 0.0d);
	// EvalTest (HOLDEM, 2, "KsAc2s", NULL, "3h5h", "Ts2c", 341, 649, 0, 34.44d, 65.56d, 0.0d, 649, 341, 0, 65.56d, 34.44d, 0.0d);
	EvalTest (HOLDEM, 2, NULL, NULL, "3h5h", "Ts2c", 788648, 889063, 34593, 46.06d, 51.92d, 2.02d, 889063, 788648, 34593, 51.92d, 46.06d, 2.02d);
	// EvalTest (OMAHA, 2, "KsAc2s3c7d", NULL, "3h5h7sAd", "Ts2c8cKc", 1, 0, 0, 100.0d, 0.0d, 0.0d, 0, 1, 0, 0.0d, 100.0d, 0.0d);
	// EvalTest (OMAHA, 2, "KsAc2s3c", NULL, "3h5h7sAd", "Ts2c8cKc", 28, 12, 0, 70.0d, 30.0d, 0.0d, 12, 28, 0, 30.0d, 70.0d, 0.0d);
	// EvalTest (OMAHA, 2, "KsAc2s", NULL, "3h5h7sAd", "Ts2c8cKc", 433, 387, 0, 52.80d, 47.20d, 0.0d, 387, 433, 0, 47.20d, 52.80d, 0.0d);
	// EvalTest (OMAHA, 2, NULL, NULL, "3h5h7sAd", "Ts2c8cKc", 261195, 238805, 0, 52.24d, 47.76d, 0.0d, 238805, 261195, 0, 47.76d, 52.24d, 0.0d);
#ifdef DEBUG
	// FsmCombinationsLog ();
	// CombinationIndexesLog ();
#endif
}

int main ()
{
	//RankTests ();
	//ComparationTests ();
	//CombinationTests ();
	EvalTests ();
}
