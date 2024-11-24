#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "poker.h"
#include "enum.h"
#include "eval.h"
#include "misc.h"
#include "fsm.h"
#include "thrdfnc.h"
#include "testing.h"

#ifdef DEBUG
#include "debug.h"
#endif

#define EPSILON .005

struct data_t
{
	int wins;
	int loses;
	int ties;
	double eqWin;
	double eqLose;
	double eqTie;
};

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

	PrintInt (hr, result.handRank, "Board: %s. Expected: %s. Actual: %s",
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

	PrintInt (expected, actual, "1st. board: %s. 2nd. board: %s. Expected: %d. Actual: %d",
		boardString1, boardString2, expected, actual);
}

bool CompareEquities (void* expected, void* actual)
{
	struct data_t* eData = (struct data_t*) expected;
	struct data_t* aData = (struct data_t*) actual;

	return eData->wins == aData->wins
		&& eData->loses == aData->loses
		&& eData->ties == aData->ties
		&& eData->eqWin - EPSILON <= aData->eqWin && aData->eqWin <= eData->eqWin + EPSILON
		&& eData->eqLose - EPSILON <= aData->eqLose && aData->eqLose <= eData->eqLose + EPSILON
		&& eData->eqTie - EPSILON <= aData->eqTie && aData->eqTie <= eData->eqTie + EPSILON;
}

void EvalTest (enum rules_t rules, int nPlayers, char* boardCards, char* deadCards, ...)
{
	va_list valist;
	struct eval_t evalData;
	struct data_t data[MAX_PLAYERS];
	struct data_t actual;
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

	for (int i = 0; i < nPlayers; i++)
	{
		data[i].wins = va_arg (valist, int);
		data[i].loses = va_arg (valist, int);
		data[i].ties = va_arg (valist, int);
		data[i].eqWin = va_arg (valist, double);
		data[i].eqLose = va_arg (valist, double);
		data[i].eqTie = va_arg (valist, double);
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

		evalData.equities[i].winProbability *= 100.0d;
		evalData.equities[i].loseProbability *= 100.0d;
		evalData.equities[i].tieProbability *= 100.0d;

		actual.wins = evalData.equities[i].wins;
		actual.loses = loses;
		actual.ties = evalData.equities[i].ties;
		actual.eqWin = evalData.equities[i].winProbability;
		actual.eqLose = evalData.equities[i].loseProbability;
		actual.eqTie = evalData.equities[i].tieProbability;

		PrintAny (
			&data[i],
			&actual,
			CompareEquities,
			"Expected: %i/%i/%i (%2.2f/%2.2f/%2.2f) Actual: %i/%i/%i (%2.2f/%2.2f/%2.2f)",
			data[i].wins,
			data[i].loses,
			data[i].ties,
			data[i].eqWin,
			data[i].eqLose,
			data[i].eqTie,
			evalData.equities[i].wins,
			loses,
			evalData.equities[i].ties,
			evalData.equities[i].winProbability,
			evalData.equities[i].loseProbability,
			evalData.equities[i].tieProbability
			);
	}
}

#ifdef DEBUG
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

	PrintInts (expected, indexes, BOARD_SIZE, "Expected: %d-%d-%d-%d-%d. Actual: %d-%d-%d-%d-%d",
		expected[4], expected[3], expected[2], expected[1], expected[0], indexes[4], indexes[3], indexes[2], indexes[1], indexes[0]);
}
#endif

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

#ifdef DEBUG
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
	IndexInitializationTest (indexes, 2, 780, 45, 0, 0, 0, 39, 38);
	IndexInitializationTest (indexes, 2, 499, 52, 0, 0, 0, 32, 2);
	IndexInitializationTest (indexes, 2, 108, 52, 0, 0, 0, 15, 2);

	IndexInitializationTest (indexes, 1, 46, 52, 0, 0, 0, 0, 45);
	IndexInitializationTest (indexes, 1, 2, 52, 0, 0, 0, 0, 1);
}
#endif

void EvalTests ()
{
	EvalTest (HOLDEM, 2, NULL, NULL, "Ah3d", "Qd7d", 960862, 742469, 8973, 56.12d, 43.36d, 0.52d, 742469, 960862, 8973, 43.36d, 56.12d, 0.52d);
	EvalTest (HOLDEM, 2, "JdAs7c", NULL, "Ac2d", "3hQs", 959, 31, 0, 96.87d, 3.13d, 0.00d, 31, 959, 0, 3.13d, 96.87d, 0.00d);
	EvalTest (HOLDEM, 2, "Kd5h4d8c", NULL, "Ks6c", "3s8s", 39, 5, 0, 88.64d, 11.36d, 0.00d, 5, 39, 0, 11.36d, 88.64d, 0.00d);
	EvalTest (HOLDEM, 2, "Td4s7h6h9c", NULL, "AdJh", "8d9s", 0, 1, 0, 0.00d, 100.00d, 0.00d, 1, 0, 0, 100.00d, 0.00d, 0.00d);
	EvalTest (HOLDEM, 3, NULL, NULL, "Kc9h", "2h2s", "5dQc", 536543, 825104, 9107, 39.14d, 60.19d, 0.66d, 434209, 927438, 9107, 31.68d, 67.66d, 0.66d, 390895, 970752, 9107, 28.52d, 70.82d, 0.66d);
	EvalTest (HOLDEM, 3, "3c6dTc", NULL, "QhKh", "5s9d", "7sTh", 235, 668, 0, 26.02d, 73.98d, 0.00d, 55, 848, 0, 6.09d, 93.91d, 0.00d, 613, 290, 0, 67.88d, 32.12d, 0.00d);
	EvalTest (HOLDEM, 3, "6sJcJs5c", NULL, "8h4h", "2c4c", "Ts8s", 3, 39, 0, 7.14d, 92.86d, 0.00d, 13, 29, 0, 30.95d, 69.05d, 0.00d, 26, 16, 0, 61.90d, 38.10d, 0.00d);
	EvalTest (HOLDEM, 3, "AsQd9cKs3d", NULL, "9h5d", "3c6h", "2hKd", 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 1, 0, 0, 100.00d, 0.00d, 0.00d);
	EvalTest (HOLDEM, 4, NULL, NULL, "7dAd", "8c9d", "AhTh", "AcJd", 149592, 890843, 45573, 13.77d, 82.03d, 4.20d, 301337, 781388, 3283, 27.75d, 71.95d, 0.30d, 275654, 764781, 45573, 25.38d, 70.42d, 4.20d, 313852, 726583, 45573, 28.90d, 66.90d, 4.20d);
	EvalTest (HOLDEM, 4, "2dQh7c", NULL, "3h5h", "7sTc", "Qs2s", "3sKh", 60, 760, 0, 7.32d, 92.68d, 0.00d, 72, 748, 0, 8.78d, 91.22d, 0.00d, 678, 142, 0, 82.68d, 17.32d, 0.00d, 10, 810, 0, 1.22d, 98.78d, 0.00d);
	EvalTest (HOLDEM, 4, "4d4s5sJh", NULL, "6c6d", "7hTd", "8dQc", "Kc9s", 22, 18, 0, 55.00d, 45.00d, 0.00d, 6, 34, 0, 15.00d, 85.00d, 0.00d, 6, 34, 0, 15.00d, 85.00d, 0.00d, 6, 34, 0, 15.00d, 85.00d, 0.00d);
	EvalTest (HOLDEM, 4, "9d7sJdJcQd", NULL, "Ac5h", "6c3c", "6h4d", "7cTd", 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 1, 0, 0, 100.00d, 0.00d, 0.00d);
	EvalTest (HOLDEM, 5, NULL, NULL, "Ts8d", "6s2d", "4hKc", "JhKs", "6d4c", 256225, 590722, 3721, 30.12d, 69.44d, 0.44d, 79024, 730835, 40809, 9.29d, 85.91d, 4.80d, 54549, 735625, 60494, 6.41d, 86.48d, 7.11d, 291013, 513307, 46348, 34.21d, 60.34d, 5.45d, 72275, 723438, 54955, 8.50d, 85.04d, 6.46d);
	EvalTest (HOLDEM, 5, "5dAs9c", NULL, "Ad3s", "ThJs", "5sQh", "7dQs", "7h8h", 460, 281, 0, 62.08d, 37.92d, 0.00d, 26, 715, 0, 3.51d, 96.49d, 0.00d, 92, 649, 0, 12.42d, 87.58d, 0.00d, 5, 724, 12, 0.67d, 97.71d, 1.62d, 146, 583, 12, 19.70d, 78.68d, 1.62d);
	EvalTest (HOLDEM, 5, "2s9sQcKd", NULL, "5cTc", "8cAh", "2hKh", "2c8s", "4s3d", 4, 34, 0, 10.53d, 89.47d, 0.00d, 0, 38, 0, 0.00d, 100.00d, 0.00d, 34, 4, 0, 89.47d, 10.53d, 0.00d, 0, 38, 0, 0.00d, 100.00d, 0.00d, 0, 38, 0, 0.00d, 100.00d, 0.00d);
	EvalTest (HOLDEM, 5, "3h9h2c3d7d", NULL, "7c8d", "Ah6d", "Kh4s", "QhQc", "3s9c", 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 1, 0, 0, 100.00d, 0.00d, 0.00d);
	EvalTest (HOLDEM, 6, NULL, NULL, "5c5d", "5s2h", "Ts6h", "7hAs", "QdJc", "KcTd", 74121, 568778, 15109, 11.26d, 86.44d, 2.30d, 48063, 594836, 15109, 7.30d, 90.40d, 2.30d, 55545, 588432, 14031, 8.44d, 89.43d, 2.13d, 156265, 501086, 657, 23.75d, 76.15d, 0.10d, 161017, 496334, 657, 24.47d, 75.43d, 0.10d, 134514, 509463, 14031, 20.44d, 77.43d, 2.13d);
	EvalTest (HOLDEM, 6, "KdJsQs", NULL, "8cJh", "4d4c", "4h8s", "TcKs", "9dAd", "9s2d", 80, 578, 8, 12.01d, 86.79d, 1.20d, 23, 635, 8, 3.45d, 95.35d, 1.20d, 0, 658, 8, 0.00d, 98.80d, 1.20d, 452, 206, 8, 67.87d, 30.93d, 1.20d, 100, 558, 8, 15.02d, 83.78d, 1.20d, 3, 655, 8, 0.45d, 98.35d, 1.20d);
	EvalTest (HOLDEM, 6, "JdAc8h6c", NULL, "6s7s", "5h3c", "2sTh", "Ts6h", "7dAd", "9d9s", 0, 35, 1, 0.00d, 97.22d, 2.78d, 0, 36, 0, 0.00d, 100.00d, 0.00d, 0, 36, 0, 0.00d, 100.00d, 0.00d, 2, 33, 1, 5.56d, 91.67d, 2.78d, 31, 5, 0, 86.11d, 13.89d, 0.00d, 2, 34, 0, 5.56d, 94.44d, 0.00d);
	EvalTest (HOLDEM, 6, "2hKd9hQdTd", NULL, "KcJh", "Js8c", "3hAh", "5cQc", "7c3d", "5dKh", 0, 0, 1, 0.00d, 0.00d, 100.00d, 0, 0, 1, 0.00d, 0.00d, 100.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d);
	EvalTest (HOLDEM, 7, NULL, NULL, "Qs8d", "As8s", "Jc9c", "4sTc", "7h2d", "Qh6d", "2c4c", 31919, 447840, 22183, 6.36d, 89.22d, 4.42d, 123827, 370209, 7906, 24.67d, 73.76d, 1.58d, 133289, 368118, 535, 26.55d, 73.34d, 0.11d, 59770, 433221, 8951, 11.91d, 86.31d, 1.78d, 42068, 454286, 5588, 8.38d, 90.51d, 1.11d, 46439, 440691, 14812, 9.25d, 87.80d, 2.95d, 28978, 458960, 14004, 5.77d, 91.44d, 2.79d);
	EvalTest (HOLDEM, 7, "5s4d4h", NULL, "3sKs", "QdTc", "9h3c", "9s5d", "Jd4s", "2d6s", "4c9d", 18, 545, 32, 3.03d, 91.60d, 5.38d, 6, 589, 0, 1.01d, 98.99d, 0.00d, 0, 563, 32, 0.00d, 94.62d, 5.38d, 67, 528, 0, 11.26d, 88.74d, 0.00d, 317, 224, 54, 53.28d, 37.65d, 9.08d, 72, 523, 0, 12.10d, 87.90d, 0.00d, 29, 512, 54, 4.87d, 86.05d, 9.08d);
	EvalTest (HOLDEM, 7, "3hQsQc7d", NULL, "AdTh", "Jc6c", "TdQh", "2cKd", "KcAh", "Jh3d", "6d8d", 0, 34, 0, 0.00d, 100.00d, 0.00d, 0, 34, 0, 0.00d, 100.00d, 0.00d, 34, 0, 0, 100.00d, 0.00d, 0.00d, 0, 34, 0, 0.00d, 100.00d, 0.00d, 0, 34, 0, 0.00d, 100.00d, 0.00d, 0, 34, 0, 0.00d, 100.00d, 0.00d, 0, 34, 0, 0.00d, 100.00d, 0.00d);
	EvalTest (HOLDEM, 7, "5h7c2s8hAc", NULL, "7h5c", "JsKh", "6h7s", "Ts9c", "2h8s", "As8c", "Qh2d", 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 1, 0, 0, 100.00d, 0.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d);
	EvalTest (HOLDEM, 8, NULL, NULL, "Td9d", "2c5d", "3c6s", "JcKd", "Qd3s", "Ks4h", "4dAd", "3hKc", 94307, 282117, 568, 25.02d, 74.83d, 0.15d, 46199, 330225, 568, 12.25d, 87.59d, 0.15d, 30250, 343463, 3279, 8.02d, 91.11d, 0.87d, 53036, 318758, 5198, 14.07d, 84.55d, 1.38d, 43552, 330161, 3279, 11.55d, 87.58d, 0.87d, 16996, 352610, 7386, 4.51d, 93.53d, 1.96d, 76888, 297348, 2756, 20.40d, 78.87d, 0.73d, 5667, 363416, 7909, 1.50d, 96.40d, 2.10d);
	EvalTest (HOLDEM, 8, "9s4cAh", NULL, "7dJd", "9h3d", "Qc8d", "6dTc", "Jh4s", "Qs6c", "Th5s", "5cKd", 15, 507, 6, 2.84d, 96.02d, 1.14d, 245, 283, 0, 46.40d, 53.60d, 0.00d, 31, 486, 11, 5.87d, 92.05d, 2.08d, 21, 494, 13, 3.98d, 93.56d, 2.46d, 98, 424, 6, 18.56d, 80.30d, 1.14d, 5, 512, 11, 0.95d, 96.97d, 2.08d, 4, 499, 25, 0.76d, 94.51d, 4.73d, 67, 449, 12, 12.69d, 85.04d, 2.27d);
	EvalTest (HOLDEM, 8, "Td8s8h7s", NULL, "3h5d", "Kc2c", "6sAc", "5h4d", "4hQh", "Jc9c", "AsJs", "Kh2s", 0, 32, 0, 0.00d, 100.00d, 0.00d, 0, 32, 0, 0.00d, 100.00d, 0.00d, 0, 32, 0, 0.00d, 100.00d, 0.00d, 0, 32, 0, 0.00d, 100.00d, 0.00d, 0, 32, 0, 0.00d, 100.00d, 0.00d, 23, 7, 2, 71.88d, 21.88d, 6.25d, 7, 23, 2, 21.88d, 71.88d, 6.25d, 0, 32, 0, 0.00d, 100.00d, 0.00d);
	EvalTest (HOLDEM, 8, "3cKs8c3s7c", NULL, "9d6h", "Ts2h", "7h2d", "AdQd", "6dJd", "3h7d", "QhKd", "5c4s", 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 1, 0, 0, 100.00d, 0.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d);
	EvalTest (HOLDEM, 9, NULL, NULL, "TdJs", "8dJc", "9c7s", "3dAs", "5d9s", "4cQc", "5sAh", "Jh2c", "5h8h", 45966, 227229, 5061, 16.52d, 81.66d, 1.82d, 13166, 256787, 8303, 4.73d, 92.28d, 2.98d, 26948, 245844, 5464, 9.68d, 88.35d, 1.96d, 24468, 238006, 15782, 8.79d, 85.53d, 5.67d, 6165, 263762, 8329, 2.22d, 94.79d, 2.99d, 69343, 208604, 309, 24.92d, 74.97d, 0.11d, 13569, 246040, 18647, 4.88d, 88.42d, 6.70d, 12711, 260484, 5061, 4.57d, 93.61d, 1.82d, 34124, 237716, 6416, 12.26d, 85.43d, 2.31d);
	EvalTest (HOLDEM, 9, "Tc4hQs", NULL, "2s8s", "Th6s", "AcKh", "9h6c", "4dKc", "5cTd", "Qh7h", "JsAh", "7sAd", 32, 433, 0, 6.88d, 93.12d, 0.00d, 39, 417, 9, 8.39d, 89.68d, 1.94d, 84, 372, 9, 18.06d, 80.00d, 1.94d, 9, 456, 0, 1.94d, 98.06d, 0.00d, 44, 418, 3, 9.46d, 89.89d, 0.65d, 42, 414, 9, 9.03d, 89.03d, 1.94d, 143, 322, 0, 30.75d, 69.25d, 0.00d, 52, 407, 6, 11.18d, 87.53d, 1.29d, 2, 457, 6, 0.43d, 98.28d, 1.29d);
	EvalTest (HOLDEM, 9, "Jc3d2cQc", NULL, "3s6h", "8c5d", "9dKs", "Ts4c", "5h9s", "9cQd", "As7d", "2hJh", "7c3h", 0, 30, 0, 0.00d, 100.00d, 0.00d, 0, 30, 0, 0.00d, 100.00d, 0.00d, 2, 28, 0, 6.67d, 93.33d, 0.00d, 0, 30, 0, 0.00d, 100.00d, 0.00d, 0, 30, 0, 0.00d, 100.00d, 0.00d, 9, 21, 0, 30.00d, 70.00d, 0.00d, 0, 30, 0, 0.00d, 100.00d, 0.00d, 19, 11, 0, 63.33d, 36.67d, 0.00d, 0, 30, 0, 0.00d, 100.00d, 0.00d);
	EvalTest (HOLDEM, 9, "Kd3c6d2d4s", NULL, "8d8h", "5sJd", "TdQs", "4hJs", "Qd2c", "KhQh", "Ad3h", "TsJc", "2h6s", 0, 1, 0, 0.00d, 100.00d, 0.00d, 1, 0, 0, 100.00d, 0.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d);
	EvalTest (HOLDEM, 10, NULL, NULL, "KcAc", "9d4d", "7h8c", "AhTh", "7d5d", "5h5c", "7c9c", "6h3d", "KsQc", "8sTc", 30784, 166993, 3599, 15.29d, 82.93d, 1.79d, 27694, 171111, 2571, 13.75d, 84.97d, 1.28d, 6080, 192503, 2793, 3.02d, 95.59d, 1.39d, 25708, 171168, 4500, 12.77d, 85.00d, 2.23d, 8392, 190089, 2895, 4.17d, 94.40d, 1.44d, 17657, 181686, 2033, 8.77d, 90.22d, 1.01d, 7491, 190452, 3433, 3.72d, 94.58d, 1.70d, 23207, 178037, 132, 11.52d, 88.41d, 0.07d, 31370, 168196, 1810, 15.58d, 83.52d, 0.90d, 9814, 187052, 4510, 4.87d, 92.89d, 2.24d);
	EvalTest (HOLDEM, 10, "2s3sJh", NULL, "9s9h", "6c4c", "As7s", "Ah5h", "QsTc", "Ac9c", "7c2d", "QdAd", "5cTs", "6d5s", 86, 320, 0, 21.18d, 78.82d, 0.00d, 21, 383, 2, 5.17d, 94.33d, 0.49d, 124, 282, 0, 30.54d, 69.46d, 0.00d, 34, 371, 1, 8.37d, 91.38d, 0.25d, 25, 381, 0, 6.16d, 93.84d, 0.00d, 0, 406, 0, 0.00d, 100.00d, 0.00d, 52, 354, 0, 12.81d, 87.19d, 0.00d, 26, 380, 0, 6.40d, 93.60d, 0.00d, 0, 405, 1, 0.00d, 99.75d, 0.25d, 35, 368, 3, 8.62d, 90.64d, 0.74d);
	EvalTest (HOLDEM, 10, "2c4d8h7d", NULL, "3d5d", "6h3c", "KhJc", "Th2h", "Kd3h", "Qc8c", "4sTd", "Qh8d", "8sJs", "7hKc", 11, 17, 0, 39.29d, 60.71d, 0.00d, 3, 25, 0, 10.71d, 89.29d, 0.00d, 0, 28, 0, 0.00d, 100.00d, 0.00d, 1, 27, 0, 3.57d, 96.43d, 0.00d, 0, 28, 0, 0.00d, 100.00d, 0.00d, 0, 23, 5, 0.00d, 82.14d, 17.86d, 4, 24, 0, 14.29d, 85.71d, 0.00d, 0, 23, 5, 0.00d, 82.14d, 17.86d, 1, 27, 0, 3.57d, 96.43d, 0.00d, 3, 25, 0, 10.71d, 89.29d, 0.00d);
	EvalTest (HOLDEM, 10, "9dJd6sKs4h", NULL, "Jc2h", "2c8s", "4c4s", "Qs8h", "AcJs", "4dJh", "3s3c", "2d3d", "Kc9s", "TcKd", 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 1, 0, 0, 100.00d, 0.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d);

	EvalTest (OMAHA, 2, "Kc5h9c", NULL, "6d2s9hKd", "TdAhThJh", 558, 262, 0, 68.05d, 31.95d, 0.00d, 262, 558, 0, 31.95d, 68.05d, 0.00d);
	EvalTest (OMAHA, 2, "QsJcKs9s", NULL, "Jd3s6cTs", "Tc7h5s5d", 40, 0, 0, 100.00d, 0.00d, 0.00d, 0, 40, 0, 0.00d, 100.00d, 0.00d);
	EvalTest (OMAHA, 2, "3d2h7c4c4s", NULL, "3h6s6h8s", "3c7sJs9d", 0, 1, 0, 0.00d, 100.00d, 0.00d, 1, 0, 0, 100.00d, 0.00d, 0.00d);
	EvalTest (OMAHA, 3, "2cAc5c", NULL, "8hAd4d4h", "8cQdKhAs", "7dQc8d2d", 208, 435, 23, 31.23d, 65.32d, 3.45d, 315, 328, 23, 47.30d, 49.25d, 3.45d, 120, 546, 0, 18.02d, 81.98d, 0.00d);
	EvalTest (OMAHA, 3, "QhTcQsKh", NULL, "9sKdTh4d", "KcAc6c4c", "Jd9h8d6d", 3, 33, 0, 8.33d, 91.67d, 0.00d, 3, 33, 0, 8.33d, 91.67d, 0.00d, 30, 6, 0, 83.33d, 16.67d, 0.00d);
	EvalTest (OMAHA, 3, "7d5d9c2sAh", NULL, "Ks9dJh3c", "8c8sJc8h", "Js3d7cAd", 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 1, 0, 0, 100.00d, 0.00d, 0.00d);
	EvalTest (OMAHA, 4, "4h4s3h", NULL, "6s3s2dQd", "2c5cTd7s", "TsQc2h5h", "7h5s6hAs", 99, 427, 2, 18.75d, 80.87d, 0.38d, 0, 447, 81, 0.00d, 84.66d, 15.34d, 53, 438, 37, 10.04d, 82.95d, 7.01d, 293, 189, 46, 55.49d, 35.80d, 8.71d);
	EvalTest (OMAHA, 4, "Ac6c7hKc", NULL, "Ts9hTh6h", "5hTc6d5d", "Ks4c7d2s", "QcQhQdAs", 5, 26, 1, 15.62d, 81.25d, 3.12d, 2, 29, 1, 6.25d, 90.62d, 3.12d, 21, 11, 0, 65.62d, 34.38d, 0.00d, 3, 29, 0, 9.38d, 90.62d, 0.00d);
	EvalTest (OMAHA, 4, "Jd4h8dJc7c", NULL, "8h4d4s3h", "9dJhKdTd", "2h3cQsAh", "2c6s3s3d", 1, 0, 0, 100.00d, 0.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d);
	EvalTest (OMAHA, 5, "AdKh9c", NULL, "Js2d8c5s", "8s9s7s5c", "9h5h2c6s", "Ah4hTcAc", "JcTsThTd", 0, 388, 18, 0.00d, 95.57d, 4.43d, 8, 391, 7, 1.97d, 96.31d, 1.72d, 0, 395, 11, 0.00d, 97.29d, 2.71d, 305, 94, 7, 75.12d, 23.15d, 1.72d, 68, 331, 7, 16.75d, 81.53d, 1.72d);
	EvalTest (OMAHA, 5, "3h8d6d7d", NULL, "Kc2h3d3s", "QsKs9dJh", "As3c6hQd", "4s5dQc6c", "4dKd4c7h", 5, 23, 0, 17.86d, 82.14d, 0.00d, 0, 28, 0, 0.00d, 100.00d, 0.00d, 1, 27, 0, 3.57d, 96.43d, 0.00d, 0, 28, 0, 0.00d, 100.00d, 0.00d, 22, 6, 0, 78.57d, 21.43d, 0.00d);
	EvalTest (OMAHA, 5, "7cJdQh2s8h", NULL, "9hKdTs5c", "3dTc5d4d", "7sJs2c2d", "JhQs4h7d", "Kc9s8s4c", 1, 0, 0, 100.00d, 0.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d);
	EvalTest (OMAHA, 6, "7h6cKh", NULL, "3cAs2hQd", "JcKsAh8d", "9c6h4s6s", "Th5sQc8c", "9d5hTd6d", "Ad3h3sAc", 6, 294, 0, 2.00d, 98.00d, 0.00d, 23, 277, 0, 7.67d, 92.33d, 0.00d, 154, 141, 5, 51.33d, 47.00d, 1.67d, 70, 226, 4, 23.33d, 75.33d, 1.33d, 35, 256, 9, 11.67d, 85.33d, 3.00d, 3, 297, 0, 1.00d, 99.00d, 0.00d);
	EvalTest (OMAHA, 6, "Tc7h4s8h", NULL, "Qh6d2sJc", "9sAs9hTh", "6h3c5hTd", "2dQs7dQd", "3dJd7s5d", "Ks6cAc8s", 2, 22, 0, 8.33d, 91.67d, 0.00d, 7, 17, 0, 29.17d, 70.83d, 0.00d, 13, 9, 2, 54.17d, 37.50d, 8.33d, 0, 24, 0, 0.00d, 100.00d, 0.00d, 0, 24, 0, 0.00d, 100.00d, 0.00d, 0, 22, 2, 0.00d, 91.67d, 8.33d);
	EvalTest (OMAHA, 6, "8d4dJhJs9c", NULL, "3hKh5cQc", "5s7cKd2c", "9d2h4h8c", "Kc6s3sAh", "4cAdTsJd", "Td7s7dTc", 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 1, 0, 0, 100.00d, 0.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d);
	EvalTest (OMAHA, 7, "5d2d6h", NULL, "2s3dQdJc", "Th9hAs6c", "QhQs8h3c", "6d9s8s7h", "AcKs5h4s", "4dJhQcJs", "4c6s8d7d", 90, 120, 0, 42.86d, 57.14d, 0.00d, 8, 201, 1, 3.81d, 95.71d, 0.48d, 9, 201, 0, 4.29d, 95.71d, 0.00d, 36, 152, 22, 17.14d, 72.38d, 10.48d, 29, 181, 0, 13.81d, 86.19d, 0.00d, 4, 206, 0, 1.90d, 98.10d, 0.00d, 12, 177, 21, 5.71d, 84.29d, 10.00d);
	EvalTest (OMAHA, 7, "2hKhTdKc", NULL, "2cKd5c3s", "Tc9d8c5s", "Ah9c4hAd", "Ts7c7s3h", "JdQh6c8s", "2d4c8hJh", "Th3d7hJc", 18, 2, 0, 90.00d, 10.00d, 0.00d, 0, 20, 0, 0.00d, 100.00d, 0.00d, 2, 18, 0, 10.00d, 90.00d, 0.00d, 0, 20, 0, 0.00d, 100.00d, 0.00d, 0, 20, 0, 0.00d, 100.00d, 0.00d, 0, 20, 0, 0.00d, 100.00d, 0.00d, 0, 20, 0, 0.00d, 100.00d, 0.00d);
	EvalTest (OMAHA, 7, "8dQdAsQc2s", NULL, "6h5h4d7d", "Ac6s5d9h", "9sQsKsJs", "4s3c6dTh", "2c5s9cAh", "3h7c8c9d", "KdJdTsKc", 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 1, 0, 0, 100.00d, 0.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d);
	EvalTest (OMAHA, 8, "Td4c3s", NULL, "5cJcAdJh", "7s4hKh8s", "3dTc2d6c", "8hQh7h2h", "8dAc6sAs", "9sKsQdAh", "Js5sTh2s", "5d6d8cQs", 16, 120, 0, 11.76d, 88.24d, 0.00d, 18, 114, 4, 13.24d, 83.82d, 2.94d, 50, 85, 1, 36.76d, 62.50d, 0.74d, 0, 132, 4, 0.00d, 97.06d, 2.94d, 0, 130, 6, 0.00d, 95.59d, 4.41d, 13, 123, 0, 9.56d, 90.44d, 0.00d, 7, 128, 1, 5.15d, 94.12d, 0.74d, 20, 108, 8, 14.71d, 79.41d, 5.88d);
	EvalTest (OMAHA, 8, "3c9cJdKd", NULL, "Qc7d4d2c", "6h7c4sTs", "Kc9h5h3h", "9dTdQhAc", "8c2dJc3s", "4hQs5cTc", "7h3dAdTh", "Kh9s4c6d", 1, 15, 0, 6.25d, 93.75d, 0.00d, 0, 16, 0, 0.00d, 100.00d, 0.00d, 0, 15, 1, 0.00d, 93.75d, 6.25d, 1, 6, 9, 6.25d, 37.50d, 56.25d, 2, 14, 0, 12.50d, 87.50d, 0.00d, 0, 7, 9, 0.00d, 43.75d, 56.25d, 2, 14, 0, 12.50d, 87.50d, 0.00d, 0, 15, 1, 0.00d, 93.75d, 6.25d);
	EvalTest (OMAHA, 8, "8d6sKsAh8s", NULL, "2s7sQd8h", "Js2h5dAs", "Jh5s6cJc", "4c8cQcTs", "Kd2d4h2c", "7cQs9c4d", "Jd3dThTc", "QhKcTd9h", 0, 1, 0, 0.00d, 100.00d, 0.00d, 1, 0, 0, 100.00d, 0.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d);
	EvalTest (OMAHA, 9, "9s7h3s", NULL, "6h7d5c9d", "6dKhAd3c", "3h4sAc5h", "4c7c8dKd", "TsTdJh8h", "Th9c2h2s", "6cJcQdAh", "KcJs9hQc", "4d4h2cJd", 11, 67, 0, 14.10d, 85.90d, 0.00d, 3, 72, 3, 3.85d, 92.31d, 3.85d, 5, 70, 3, 6.41d, 89.74d, 3.85d, 2, 76, 0, 2.56d, 97.44d, 0.00d, 34, 44, 0, 43.59d, 56.41d, 0.00d, 4, 74, 0, 5.13d, 94.87d, 0.00d, 2, 73, 3, 2.56d, 93.59d, 3.85d, 9, 66, 3, 11.54d, 84.62d, 3.85d, 2, 76, 0, 2.56d, 97.44d, 0.00d);
	EvalTest (OMAHA, 9, "5d3dKsTc", NULL, "8sAs2d8c", "5s6sQhQs", "7s7h9h8d", "QdJc8h3s", "Jh7c5h4h", "9d5c7d4d", "Jd3c6h2h", "9c6d2cAc", "Ad6cQc9s", 0, 12, 0, 0.00d, 100.00d, 0.00d, 4, 8, 0, 33.33d, 66.67d, 0.00d, 0, 12, 0, 0.00d, 100.00d, 0.00d, 2, 10, 0, 16.67d, 83.33d, 0.00d, 0, 12, 0, 0.00d, 100.00d, 0.00d, 2, 10, 0, 16.67d, 83.33d, 0.00d, 1, 9, 2, 8.33d, 75.00d, 16.67d, 0, 10, 2, 0.00d, 83.33d, 16.67d, 1, 11, 0, 8.33d, 91.67d, 0.00d);
	EvalTest (OMAHA, 9, "2sKcAhJsTh", NULL, "Ts4cKdTd", "4sKh3h9s", "8sTcQs6d", "5dJd2d2h", "5hAdAc8d", "7dQc6h5c", "Jc3d9cKs", "6cJh9h7s", "9d5s4dAs", 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 1, 0, 0, 100.00d, 0.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d);
	EvalTest (OMAHA, 10, "3c7h7c", NULL, "8h8c2cQh", "Qd6s3s4h", "9dTsQs9c", "3hTdKhKc", "4d2d3dAd", "2sAs2h5d", "4sAh6hJs", "9sKd8d4c", "AcTcTh6d", "5c5sJd9h", 7, 29, 0, 19.44d, 80.56d, 0.00d, 0, 36, 0, 0.00d, 100.00d, 0.00d, 0, 36, 0, 0.00d, 100.00d, 0.00d, 19, 17, 0, 52.78d, 47.22d, 0.00d, 0, 36, 0, 0.00d, 100.00d, 0.00d, 0, 36, 0, 0.00d, 100.00d, 0.00d, 0, 36, 0, 0.00d, 100.00d, 0.00d, 0, 36, 0, 0.00d, 100.00d, 0.00d, 6, 30, 0, 16.67d, 83.33d, 0.00d, 4, 32, 0, 11.11d, 88.89d, 0.00d);
	EvalTest (OMAHA, 10, "8sKsQc6c", NULL, "7sJc7dJh", "5h5c8h7c", "Td2d5s6d", "Ad2sTsTc", "KdQh3c6h", "8c5dQdTh", "4h9h6s9d", "3d2c7h4d", "AhAc9sKc", "8d3s3hJs", 0, 8, 0, 0.00d, 100.00d, 0.00d, 0, 8, 0, 0.00d, 100.00d, 0.00d, 0, 8, 0, 0.00d, 100.00d, 0.00d, 1, 7, 0, 12.50d, 87.50d, 0.00d, 3, 5, 0, 37.50d, 62.50d, 0.00d, 0, 8, 0, 0.00d, 100.00d, 0.00d, 0, 8, 0, 0.00d, 100.00d, 0.00d, 0, 8, 0, 0.00d, 100.00d, 0.00d, 2, 6, 0, 25.00d, 75.00d, 0.00d, 2, 6, 0, 25.00d, 75.00d, 0.00d);
	EvalTest (OMAHA, 10, "4cKhAs4sJd", NULL, "9c2hQs7c", "KcJh3c2s", "JsAcTd5h", "8d9s6d6c", "3h9dQh8s", "Ks7s5s4h", "8cJcAdQc", "Th7hTc3s", "Ts2c7d2d", "5d4d6h6s", 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 1, 0, 0, 100.00d, 0.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d, 0, 1, 0, 0.00d, 100.00d, 0.00d);
}

int main ()
{
	//RankTests ();
	//ComparationTests ();
#ifdef DEBUG
	//IndexInitializationTests ();
#endif
	EvalTests ();
}
