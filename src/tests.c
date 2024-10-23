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
#define SCREEN_COLUMNS 80
#define PASSED_MSG "PASSED"
#define FAILED_MSG "FAILED"

struct eq_t
{
	int wins;
	int loses;
	int ties;
	double eqWin;
	double eqLose;
	double eqTie;
};

bool CompareStrings (void* value1, void* value2, int length)
{
	char* v1 = (char*) value1;
	char* v2 = (char*) value2;

	if (v1 == NULL)
	{
		return v2 == NULL;
	}
	else if (v2 == NULL)
	{
		return false;
	}

	int i = 0;

	while (v1[i] == v2[i] && v1[i] != '\0')
	{
		++i;
	}

	return v1[i] == v2[i];
}

bool CompareInts (void* value1, void* value2, int length)
{
	int* v1 = (int*) value1;
	int* v2 = (int*) value2;

	for (int i = 0; i < length; i++)
	{
		if (v1[i] != v2[i]) return false;
	}

	return true;
}

void Print (void* expected, void* actual, int length, bool (*compare)(void*, void*, int), char* format, ...)
{
	char buffer[SCREEN_COLUMNS];
	va_list valist;

	memset (buffer, 0, SCREEN_COLUMNS);

    va_start (valist, format);
    int written = vsnprintf (buffer, SCREEN_COLUMNS, format, valist);
    va_end (valist);

	if (written < 0 || written >= SCREEN_COLUMNS)
	{
		return;
	}

	printf ("%s%*s\n", buffer, SCREEN_COLUMNS - strlen (buffer), compare (expected, actual, length) ? PASSED_MSG : FAILED_MSG);
}

void HandRankTest (char* boardString, enum hand_rank_t hr)
{
	struct card_t board[BOARD_SIZE];
	struct hand_rank_result_t result;

	if (StringToCards (boardString, board) == NULL)
	{
		printf ("Invalid input: %s", boardString);
		return;
	}

	HandRank (board, &result);

	Print (&hr, &result.handRank, 1, CompareInts, "Board: %s. Expected: %s. Actual: %s",
		boardString, HandRankToString (hr), HandRankToString (result.handRank));
}

void CompareTest (char* boardString1, char* boardString2, int expected)
{
	struct card_t board1[BOARD_SIZE];
	struct card_t board2[BOARD_SIZE];
	struct hand_rank_result_t result1;
	struct hand_rank_result_t result2;

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

	int actual = Compare (board1, board2);

	Print (&expected, &actual, 1, CompareInts, "1st. board: %s. 2nd. board: %s. Expected: %d. Actual: %d",
		boardString1, boardString2, expected, actual);
}

void EvalTest (enum rules_t rules, int nPlayers, char* boardCards, char* deadCards, ...)
{
	va_list valist;
	struct eval_t evalData;
	struct eq_t equities[MAX_PLAYERS];
	char cardBuffer[3];

	evalData.rules = rules;
	evalData.nPlayers = nPlayers;
	StringToCards (boardCards, evalData.boardCards);
	StringToCards (deadCards, evalData.deadCards);

	evalData.nBoardCards = boardCards != NULL ? strlen (boardCards) / 2 : 0;
	evalData.nDeadCards = deadCards != NULL ? strlen (deadCards) / 2 : 0;

	va_start (valist, deadCards);

	evalData.nHoleCards = 0;

	for (int i = 0; i < nPlayers; i++)
	{
		char* holeCards = va_arg (valist, char*);
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

void IndexInitializationTest (int* indexes, int nIndexes, int nCombinations, int nCards, ...)
{
	va_list valist;
	int expected[BOARD_SIZE];

	for (int i = 0; i < BOARD_SIZE; i++)
	{
		indexes[i] = 0;
	}

	InitializeIndexes (indexes, nIndexes, nCombinations, nCards);

	va_start (valist, nCards);

	for (int i = BOARD_SIZE - 1; i >= 0; i--)
	{
		expected[i] = va_arg (valist, int);
	}

	va_end (valist);

	Print (expected, indexes, BOARD_SIZE, CompareInts, "Expected: %d-%d-%d-%d-%d. Actual: %d-%d-%d-%d-%d",
		expected[4], expected[3], expected[2], expected[1], expected[0], indexes[4], indexes[3], indexes[2], indexes[1], indexes[0]);
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

void IndexInitializationTests ()
{
	int indexes[BOARD_SIZE];

	IndexInitializationTest (indexes, 5, 2598960, 52, 51, 50, 49, 48, 47);
	IndexInitializationTest (indexes, 5, 1, 52, 4, 3, 2, 1, 0 );
	IndexInitializationTest (indexes, 5, 1299480, 52, 45, 38, 29, 22, 20);
	IndexInitializationTest (indexes, 5, 649740, 52, 39, 38, 11, 2, 1);
	IndexInitializationTest (indexes, 5, 1949220, 52, 49, 33, 21, 13, 7);

	IndexInitializationTest (indexes, 4, 230300, 52, 0, 49, 48, 47, 46);
	IndexInitializationTest (indexes, 4, 115150, 52, 0, 42, 27, 24, 18);
	IndexInitializationTest (indexes, 4, 57575, 52, 0, 35, 32, 23, 1);
	IndexInitializationTest (indexes, 4, 28787, 52, 0, 30, 21, 10, 6);
	IndexInitializationTest (indexes, 4, 14393, 52, 0, 25, 22, 20, 12);

	IndexInitializationTest (indexes, 3, 18420, 52, 0, 0, 48, 47, 42);
	IndexInitializationTest (indexes, 3, 9210, 52, 0, 0, 39, 12, 4);
	IndexInitializationTest (indexes, 3, 4600, 52, 0, 0, 31, 14, 13);
	IndexInitializationTest (indexes, 3, 555, 52, 0, 0, 15, 14, 8);

	IndexInitializationTest (indexes, 2, 1000, 52, 0, 0, 0, 45, 9);
	IndexInitializationTest (indexes, 2, 499, 52, 0, 0, 0, 32, 2);
	IndexInitializationTest (indexes, 2, 108, 52, 0, 0, 0, 15, 2);

	IndexInitializationTest (indexes, 1, 46, 52, 0, 0, 0, 0, 45);
	IndexInitializationTest (indexes, 1, 2, 52, 0, 0, 0, 0, 1);
}

void EvalTests ()
{
	//EvalTest (HOLDEM, 2, "KsAc2s3c7d", NULL, "3h5h", "Ts2c", 1, 0, 0, 100.0d, 0.0d, 0.0d, 0, 1, 0, 0.0d, 100.0d, 0.0d);
	//EvalTest (HOLDEM, 2, "KsAc2s3c", NULL, "3h5h", "Ts2c", 39, 5, 0, 88.64d, 11.36d, 0.0d, 5, 39, 0, 11.36d, 88.64d, 0.0d);
	EvalTest (HOLDEM, 2, "KsAc2s", NULL, "3h5h", "Ts2c", 341, 649, 0, 34.44d, 65.56d, 0.0d, 649, 341, 0, 65.56d, 34.44d, 0.0d);
	//EvalTest (HOLDEM, 2, NULL, NULL, "3h5h", "Ts2c", 788648, 889063, 34593, 46.06d, 51.92d, 2.02d, 889063, 788648, 34593, 51.92d, 46.06d, 2.02d);
	//EvalTest (OMAHA, 2, "KsAc2s3c7d", NULL, "3h5h7sAd", "Ts2c8cKc", 1, 0, 0, 100.0d, 0.0d, 0.0d, 0, 1, 0, 0.0d, 100.0d, 0.0d);
	//EvalTest (OMAHA, 2, "KsAc2s3c", NULL, "3h5h7sAd", "Ts2c8cKc", 28, 12, 0, 70.0d, 30.0d, 0.0d, 12, 28, 0, 30.0d, 70.0d, 0.0d);
	//EvalTest (OMAHA, 2, "KsAc2s", NULL, "3h5h7sAd", "Ts2c8cKc", 433, 387, 0, 52.80d, 47.20d, 0.0d, 387, 433, 0, 47.20d, 52.80d, 0.0d);
	//EvalTest (OMAHA, 2, NULL, NULL, "3h5h7sAd", "Ts2c8cKc", 261195, 238805, 0, 52.24d, 47.76d, 0.0d, 238805, 261195, 0, 47.76d, 52.24d, 0.0d);
}

int main ()
{
	//RankTests ();
	//ComparationTests ();
	//IndexInitializationTests ();
	EvalTests ();
}
