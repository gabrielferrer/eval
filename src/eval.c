#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifdef DEBUG
#include "debug.h"
#endif

#include "core.h"
#include "eval.h"
#include "enum.h"
#include "misc.h"
#include "fsm.h"
#include "cmbntn.h"
#include "thread.h"
#include "thrdfnc.h"

struct range_t
{
	int min;
	int max;
};

int contributions[BOARD_SIZE][DECK_SIZE] =
{
	{ 0, 1, 2, 3, 4,  5,  6,  7,  8,   9,  10,  11,  12,   13,   14,   15,   16,   17,   18,    19,    20,    21,    22,    23,    24,    25,    26,    27,    28,     29,     30,     31,     32,     33,     34,     35,     36,     37,     38,     39,     40,     41,     42,     43,      44,      45,      46,      47,      48,      49,      50,      51 },
	{ 0, 0, 1, 3, 6, 10, 15, 21, 28,  36,  45,  55,  66,   78,   91,  105,  120,  136,  153,   171,   190,   210,   231,   253,   276,   300,   325,   351,   378,    406,    435,    465,    496,    528,    561,    595,    630,    666,    703,    741,    780,    820,    861,    903,     946,     990,    1035,    1081,    1128,    1176,    1225,    1275 },
	{ 0, 0, 0, 1, 4, 10, 20, 35, 56,  84, 120, 165, 220,  286,  364,  455,  560,  680,  816,   969,  1140,  1330,  1540,  1771,  2024,  2300,  2600,  2925,  3276,   3654,   4060,   4495,   4960,   5456,   5984,   6545,   7140,   7770,   8436,   9139,   9880,  10660,  11480,  12341,   13244,   14190,   15180,   16215,   17296,   18424,   19600,   20825 },
	{ 0, 0, 0, 0, 1,  5, 15, 35, 70, 126, 210, 330, 495,  715, 1001, 1365, 1820, 2380, 3060,  3876,  4845,  5985,  7315,  8855, 10626, 12650, 14950, 17550, 20475,  23751,  27405,  31465,  35960,  40920,  46376,  52360,  58905,  66045,  73815,  82251,  91390, 101270, 111930, 123410,  135751,  148995,  163185,  178365,  194580,  211876,  230300,  249900 },
	{ 0, 0, 0, 0, 0,  1,  6, 21, 56, 126, 252, 462, 792, 1287, 2002, 3003, 4368, 6188, 8568, 11628, 15504, 20349, 26334, 33649, 42504, 53130, 65780, 80730, 98280, 118755, 142506, 169911, 201376, 237336, 278256, 324632, 376992, 435897, 501942, 575757, 658008, 749398, 850668, 962598, 1086008, 1221759, 1370754, 1533939, 1712304, 1906884, 2118760, 2349060 }
};

/*
	Returns how many combinations there are for the given poker rules.

	[Returns]

		The number of combinations.
*/
int GetPlayerCombinations (enum rules_t rules)
{
	if (rules == HOLDEM)
	{
		// For Hold'em user can form 21 combinations total.
		// Zero, one or two hole cards can be used combined with 5 cards from board.
		// C(5, 0) + 2 * C(5, 1) + C(5, 2) = 1 + 2 * 5 + 10 = 21
		return 21;
	}

	if (rules == OMAHA)
	{
		// For Omaha (4 hole cards) user can form 60 combinations total.
		// Two hole cards mandatory combined with 5 cards from board.
		// C(4, 2) * C(5, 2) = 6 * 10 = 60
		return 60;
	}

	if (rules == OMAHA5)
	{
		// For Omaha (5 hole cards) user can form 100 combinations total.
		// Two hole cards mandatory combined with 5 cards from board.
		// C(5, 2) * C(5, 2) = 10 * 10 = 100
		return 100;
	}

	if (rules == OMAHA6)
	{
		// For Omaha (6 hole cards) user can form 150 combinations total.
		// Two hole cards mandatory combined with 5 cards from board.
		// C(6, 2) * C(5, 2) = 15 * 10 = 150
		return 150;
	}

	return 0;
}

void InitializeIndexes (int* indexes, int nIndexes, int nCombinations, int nCards)
{
	struct range_t ranges[BOARD_SIZE];

	for (int i = 0; i < BOARD_SIZE; i++)
	{
		ranges[i].min = i < nIndexes ? i : 0;
		ranges[i].max = i < nIndexes ? nCards - nIndexes + i : 0;
	}

	for (int i = nIndexes - 1; i >= 0; i--)
	{
		int pivot;

		while (ranges[i].min <= ranges[i].max)
		{
			pivot = (ranges[i].min + ranges[i].max) / 2;

			if (nCombinations - contributions[i][pivot] > 1)
			{
				ranges[i].min = pivot + 1;
			}
			else if (nCombinations - contributions[i][pivot] < 1)
			{
				ranges[i].max = pivot - 1;
			}
			else
			{
				break;
			}
		}

		if (nCombinations - contributions[i][pivot] <= 0 && pivot > 0)
		{
			--pivot;
		}

		indexes[nIndexes - i - 1] = nCards - pivot - 1;
		nCombinations -= contributions[i][pivot];
	}
}

void FreeThreadInfo (thread_id_t* threadIds, struct thread_args_t* threadArgs, int nThreads, int nPlayers)
{
	if (threadArgs) free (threadArgs);

	if (threadIds)
	{
		for (int i = 0; i < nThreads; i++)
		{
			if (VALID_THREAD_ID(threadIds[i]))
			{
				TH_DisposeThread (threadIds[i]);
			}
		}

		free (threadIds);
	}
}

__declspec(dllexport)
bool Eval (struct eval_t* evalData)
{
	struct card_t deck[DECK_SIZE];
	evalData->nBoards = 0;
	evalData->errors = 0;

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		evalData->equities[i].wins = 0;
		evalData->equities[i].ties = 0;
		evalData->equities[i].winProbability = -1;
		evalData->equities[i].loseProbability = -1;
		evalData->equities[i].tieProbability = -1;
	}

	if (evalData->nBoardCards < 0 || evalData->nBoardCards == 1 || evalData->nBoardCards == 2 || evalData->nBoardCards > 5)
	{
		evalData->errors |= INVALID_BOARD_CARDS_COUNT;
		return false;
	}

	// Initialize deck.
	// It's important to traverse ranks first and then suits.
	// If not, INDEX macro will fail.
	for (enum rank_t r = TWO; r <= ACE; r++)
	{
		for (enum suit_t s = CLUBS; s <= SPADES; s++)
		{
			int i = INDEX (r, s);
			deck[i].rank = r;
			deck[i].suit = s;
		}
	}

	for (int i = 0; i < evalData->nBoardCards; i++)
	{
		int j = INDEX (evalData->boardCards[i].rank, evalData->boardCards[i].suit);

		// Check for duplicated cards.
		if (deck[j].rank == NO_RANK)
		{
			evalData->errors |= DUPLICATED_CARD_FLAG;
			return false;
		}

		deck[j].rank = NO_RANK;
		deck[j].suit = NO_SUIT;
	}

	// Remove dead cards from deck.
	for (int i = 0; i < evalData->nDeadCards; i++)
	{
		int j = INDEX (evalData->deadCards[i].rank, evalData->deadCards[i].suit);

		if (deck[j].rank == NO_RANK)
		{
			evalData->errors |= DUPLICATED_CARD_FLAG;
			return false;
		}

		deck[j].rank = NO_RANK;
		deck[j].suit = NO_SUIT;
	}

	// Remove hole cards from deck.
	for (int i = 0; i < evalData->nPlayers; i++)
	{
		for (int j = 0; j < evalData->nHoleCards; j++)
		{
			int k = INDEX (evalData->holeCards[i][j].rank, evalData->holeCards[i][j].suit);

			if (deck[k].rank == NO_RANK)
			{
				evalData->errors |= DUPLICATED_CARD_FLAG;
				return false;
			}

			deck[k].rank = NO_RANK;
			deck[k].suit = NO_SUIT;
		}
	}

	int nCards = 0;
	int source = nCards + 1;

	// Compact remaining cards.
	while (source < DECK_SIZE)
	{	
		if (deck[nCards].rank != NO_RANK)
		{
			++nCards;
		}
		else if (deck[source].rank != NO_RANK)
		{
			deck[nCards].rank = deck[source].rank;
			deck[nCards++].suit = deck[source].suit;
			deck[source].rank = NO_RANK;
			deck[source].suit = NO_SUIT;
		}

		++source;
	}

	if (nCards < BOARD_SIZE)
	{
		evalData->errors |= INSUFFICIENT_COMBINATION_CARDS;
		return false;
	}

	int playerCombinations = GetPlayerCombinations (evalData->rules);

	if (playerCombinations == 0)
	{
		evalData->errors |= INVALID_POKER_RULES;
		return false;
	}

	int combinationSize = BOARD_SIZE - evalData->nBoardCards;
	thread_id_t* threadIds = NULL;
	struct thread_args_t* threadArgs = NULL;
	int nThreads = 0;

	if (combinationSize == 0)
	{
		evalData->nBoards = nThreads = 1;

		threadIds = (thread_id_t*) malloc (nThreads * sizeof (thread_id_t));
		threadArgs = (struct thread_args_t*) malloc (nThreads * sizeof (struct thread_args_t));

		memset (threadArgs, 0, sizeof (struct thread_args_t));
#ifdef DEBUG
		threadArgs[0].threadNr = 0;
#endif
		threadArgs[0].rules = evalData->rules;
		threadArgs[0].nPlayers = evalData->nPlayers;
		threadArgs[0].nBoardCards = evalData->nBoardCards;
		threadArgs[0].nHoleCards = evalData->nHoleCards;

		memcpy (threadArgs[0].boardCards, evalData->boardCards, evalData->nBoardCards * sizeof (struct card_t));

		for (int j = 0; j < evalData->nPlayers; j++)
		{
			memcpy (threadArgs[0].holeCards[j], evalData->holeCards[j], evalData->nHoleCards * sizeof (struct card_t));
		}

		memcpy (threadArgs[0].cards, deck, nCards * sizeof (struct card_t));

		threadArgs[0].nCards = nCards;
		threadArgs[0].nCombinations = 0;
		threadArgs[0].nCombinationCards = combinationSize;

		threadIds[0] = TH_CreateThread (ThreadFunction, threadArgs);

		if (!VALID_THREAD_ID(threadIds[0]))
		{
			evalData->errors |= INTERNAL_ERROR;

			FreeThreadInfo (threadIds, threadArgs, nThreads, evalData->nPlayers);

			return false;
		}
	}
	else
	{
		int nCores = GetCores ();
		long int nCombinations = CMB_Combination (nCards, combinationSize);
		evalData->nBoards = nCombinations;

		int combinationsPerThread = nCombinations / nCores;
		int reminder = nCombinations % nCores;

		nThreads = combinationsPerThread == 0 ? reminder : nCores;

		threadIds = (thread_id_t*) malloc (nThreads * sizeof (thread_id_t));
		threadArgs = (struct thread_args_t*) malloc (nThreads * sizeof (struct thread_args_t));

		for (int i = 0; i < nThreads; i++)
		{
			memset (&threadArgs[i], 0, sizeof (struct thread_args_t));
#ifdef DEBUG
			threadArgs[i].threadNr = i;
#endif
			threadArgs[i].rules = evalData->rules;
			threadArgs[i].nPlayers = evalData->nPlayers;
			threadArgs[i].nBoardCards = evalData->nBoardCards;
			threadArgs[i].nHoleCards = evalData->nHoleCards;

			memcpy (threadArgs[i].boardCards, evalData->boardCards, evalData->nBoardCards * sizeof (struct card_t));

			for (int j = 0; j < evalData->nPlayers; j++)
			{
				memcpy (threadArgs[i].holeCards[j], evalData->holeCards[j], evalData->nHoleCards * sizeof (struct card_t));
			}

			memcpy (threadArgs[i].cards, deck, nCards * sizeof (struct card_t));

			threadArgs[i].nCards = nCards;
			threadArgs[i].nCombinations = combinationsPerThread + (i < reminder ? 1 : 0);
			threadArgs[i].nCombinationCards = combinationSize;

			InitializeIndexes (threadArgs[i].indexes, combinationSize, nCombinations, nCards);
#ifdef DEBUG
			D_WriteThreadArguments (&threadArgs[i], nCombinations, "C:\\Users\\Gabriel\\Desktop\\threadargs.txt");
#endif
			nCombinations -= threadArgs[i].nCombinations;

			threadIds[i] = TH_CreateThread (ThreadFunction, &threadArgs[i]);

			if (!VALID_THREAD_ID(threadIds[i]))
			{
				evalData->errors |= INTERNAL_ERROR;

				FreeThreadInfo (threadIds, threadArgs, nThreads, evalData->nPlayers);

				return false;
			}
		}
	}

	TH_WaitThreads (threadIds, nThreads);

	for (int i = 0; i < nThreads; i++)
	{
		for (int j = 0; j < evalData->nPlayers; j++)
		{
			evalData->equities[j].wins += threadArgs[i].results[j].wins;
			evalData->equities[j].ties += threadArgs[i].results[j].ties;
		}
	}

	FreeThreadInfo (threadIds, threadArgs, nThreads, evalData->nPlayers);

	for (int i = 0; i < evalData->nPlayers; i++)
	{
		int loses = evalData->nBoards - evalData->equities[i].wins - evalData->equities[i].ties;
		evalData->equities[i].winProbability = (double) evalData->equities[i].wins / (double) evalData->nBoards;
		evalData->equities[i].tieProbability = (double) evalData->equities[i].ties / (double) evalData->nBoards;
		evalData->equities[i].loseProbability = (double) loses / (double) evalData->nBoards;
	}

	return true;
}
