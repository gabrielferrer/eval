#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#ifdef DEBUG
#include "debug.h"
#endif

#include "eval.h"
#include "enum.h"
#include "misc.h"
#include "fsm.h"

#define PAGE_SIZE 10000

typedef struct
{
	int id;
} thread_t;

typedef struct
{
	board_t bestBoard;
	int playerIndex;
} player_info_t;

int contributions[BOARD_SIZE][DECK_SIZE] =
{
	{ 0, 1, 2, 3, 4,  5,  6,  7,  8,   9,  10,  11,  12,   13,   14,   15,   16,   17,   18,    19,    20,    21,    22,    23,    24,    25,    26,    27,    28,     29,     30,     31,     32,     33,     34,     35,     36,     37,     38,     39,     40,     41,     42,     43,      44,      45,      46,      47,       0,       0,       0,       0 },
	{ 0, 0, 1, 3, 6, 10, 15, 21, 28,  36,  45,  55,  66,   78,   91,  105,  120,  136,  153,   171,   190,   210,   231,   253,   276,   300,   325,   351,   378,    406,    435,    465,    496,    528,    561,    595,    630,    666,    703,    741,    780,    820,    861,    903,     946,     990,    1035,    1081,    1128,       0,       0,       0 },
	{ 0, 0, 0, 1, 4, 10, 20, 35, 56,  84, 120, 165, 220,  286,  364,  455,  560,  680,  816,   969,  1140,  1330,  1540,  1771,  2024,  2300,  2600,  2925,  3276,   3654,   4060,   4495,   4960,   5456,   5984,   6545,   7140,   7770,   8436,   9139,   9880,  10660,  11480,  12341,   13244,   14190,   15180,   16215,   17296,   18424,       0,       0 },
	{ 0, 0, 0, 0, 1,  5, 15, 35, 70, 126, 210, 330, 495,  715, 1001, 1365, 1820, 2380, 3060,  3876,  4845,  5985,  7315,  8855, 10626, 12650, 14950, 17550, 20475,  23751,  27405,  31465,  35960,  40920,  46376,  52360,  58905,  66045,  73815,  82251,  91390, 101270, 111930, 123410,  135751,  148995,  163185,  178365,  194580,  211876,  230300,       0 },
	{ 0, 0, 0, 0, 0,  1,  6, 21, 56, 126, 252, 462, 792, 1287, 2002, 3003, 4368, 6188, 8568, 11628, 15504, 20349, 26334, 33649, 42504, 53130, 65780, 80730, 98280, 118755, 142506, 169911, 201376, 237336, 278256, 324632, 376992, 435897, 501942, 575757, 658008, 749398, 850668, 962598, 1086008, 1221759, 1370754, 1533939, 1712304, 1906884, 2118760, 2349060 }
};

board_t tempBoard1;
board_t tempBoard2;
hand_rank_result_t tempResult1;
hand_rank_result_t tempResult2;
player_info_t tempPlayerInfo[MAX_PLAYERS];

/*
	Returns how many combinations there are for the given poker rules.

	[Returns]

		The number of combinations.
*/
int GetPlayerCombinations (rules_t rules)
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

void CalcBestBoard (rules_t rules, board_t bestBoard)
{
	board_t current;

	FSM_Next (current);
	// Initialize best board.
	memcpy (bestBoard, current, sizeof (board_t));

	while (FSM_Next (current))
	{
		int comparison = Compare (current, bestBoard);

		if (comparison <= 0)
		{
			continue;
		}

		memcpy (bestBoard, current, sizeof (board_t));
	}
}

void EvalPlayers (eval_t* evalData, board_t* boardsPage, int nPageEntries)
{
	board_t bestBoard;

	for (int i = 0; i < nPageEntries; i++)
	{
		int nBest = 0;

		FSM_ResetBoardCards (boardsPage[i]);

		for (int j = 0; j < evalData->nPlayers; j++)
		{
			FSM_ResetHoleCards (evalData->holeCards[j], evalData->nHoleCards);

			CalcBestBoard (evalData->rules, bestBoard);

			if (nBest == 0)
			{
				memcpy (tempPlayerInfo[nBest].bestBoard, bestBoard, sizeof (board_t));
				tempPlayerInfo[nBest++].playerIndex = j;
				continue;
			}

			int comparison = Compare (tempPlayerInfo[0].bestBoard, bestBoard);

			if (comparison > 0)
			{
//#ifdef DEBUG
//				D_WriteSideBySideBoards ("C:\\Users\\Gabriel\\Desktop\\log.txt", tempPlayerInfo[0].bestBoard, bestBoard);
//#endif
				continue;
			}

			if (comparison == 0)
			{
//#ifdef DEBUG
//				D_WriteSideBySideBoards ("C:\\Users\\Gabriel\\Desktop\\ties.txt", tempPlayerInfo[0].bestBoard, bestBoard);
//#endif
				memcpy (tempPlayerInfo[nBest].bestBoard, bestBoard, sizeof (board_t));
				tempPlayerInfo[nBest++].playerIndex = j;
				continue;
			}
//#ifdef DEBUG
//				D_WriteSideBySideBoards ("C:\\Users\\Gabriel\\Desktop\\log.txt", bestBoard, tempPlayerInfo[0].bestBoard);
//#endif
			nBest = 0;
			memcpy (tempPlayerInfo[nBest].bestBoard, bestBoard, sizeof (board_t));
			tempPlayerInfo[nBest++].playerIndex = j;
		}

		if (nBest == 1)
		{
			evalData->equities[tempPlayerInfo[0].playerIndex].wins++;
		}
		else
		{
			for (int k = 0; k < nBest; k++)
			{
				evalData->equities[tempPlayerInfo[k].playerIndex].ties++;
			}
		}
	}
}

int CompareCards (const void* a, const void* b)
{
   card_t* c1 = (card_t*) a;
   card_t* c2 = (card_t*) b;

   if (c1->rank < c2->rank)
   {
	   return -1;
   }

   if (c1->rank > c2->rank)
   {
	   return 1;
   }

   return 0;
}

/*
	Compare a set of cards.

	[Returns]

		-1, if first card is ranked below second card.
		0, if both cards are ranked equal.
		1, if first card is ranked above second card.
*/
int CompareSingleGroupCards (card_t* c1[], card_t* c2[], int count)
{
	for (int i = count - 1; i >= 0; i--)
	{
		if (c1[i]->rank < c2[i]->rank)
		{
			return -1;
		}
		else if (c1[i]->rank > c2[i]->rank)
		{
			return 1;
		}
	}
	
	return 0;
}

void CheckGroup (group_t* g, card_t* c, hand_rank_result_t* r)
{
	if (g->count == 1)
	{
		r->singleGroupCards[r->nSingleGroupCards++] = c;
	}
	else if (g->count == 2)
	{
		if (r->lowPair == NULL)
		{
			r->lowPair = g;
		}
		else
		{
			r->highPair = g;
		}
	}
	else if (g->count == 3)
	{
		r->trips = g;
	}
	else if (g->count == 4)
	{
		r->four = g;
	}
}

bool StraightSpecialCase (board_t b)
{
	return b[0].rank == TWO && b[1].rank == THREE && b[2].rank == FOUR && b[3].rank == FIVE && b[4].rank == ACE;
}

void ReorderStraightSpecialCase (board_t b)
{
	suit_t s = b[BOARD_SIZE - 1].suit;

	for (int i = BOARD_SIZE - 1; i > 0; i--)
	{
		b[i].rank = b[i - 1].rank;
		b[i].suit = b[i - 1].suit;
	}

	b[0].rank = ONE;
	b[0].suit = s;
}

/*
	Calculate a given board's rank.

	[Returns]

		Struct with information about the hand rank.
*/
void HandRank (board_t board, hand_rank_result_t* result)
{
	rank_t cr = NO_RANK;

	memset (result, 0, sizeof (hand_rank_result_t));
	result->nGroups = -1;
	memcpy (result->orderedCards, board, sizeof (board_t));
	qsort (result->orderedCards, BOARD_SIZE, sizeof (card_t), CompareCards);

	if (StraightSpecialCase (result->orderedCards))
	{
		ReorderStraightSpecialCase (result->orderedCards);
	}

	int i = 0;

	while (i < BOARD_SIZE)
	{
		if (i + 1 < BOARD_SIZE)
		{
			// Check consecutive ranks.
			if (result->orderedCards[i].rank + 1 == result->orderedCards[i + 1].rank)
			{
				result->nConsecutiveRanks++;
			}

			// Check same suit.
			if (result->orderedCards[i].suit == result->orderedCards[i + 1].suit)
			{
				result->nSameSuit++;
			}
		}

		if (cr != result->orderedCards[i].rank)
		{
			if (cr != NO_RANK)
			{
				CheckGroup (&result->groups[result->nGroups], &result->orderedCards[i - 1], result);
			}

			// New group.
			// Increment group index and initialize group.
			result->nGroups++;
			result->groups[result->nGroups].rank = cr = result->orderedCards[i].rank;
			result->groups[result->nGroups].count = 1;
		}
		else
		{
			result->groups[result->nGroups].count++;
		}

		i++;
	}

	// Check last group for extra information.
	CheckGroup (&result->groups[result->nGroups], &result->orderedCards[i - 1], result);
	// Increment to get how many groups there are.
	result->nGroups++;

	if (result->nGroups == 2)
	{
		if (result->groups[0].count == 4 && result->groups[1].count == 1 || result->groups[0].count == 1 && result->groups[1].count == 4)
		{
			result->handRank = FOUR_OF_A_KIND;
			return;
		}

		if (result->groups[0].count == 3 && result->groups[1].count == 2 || result->groups[0].count == 2 && result->groups[1].count == 3)
		{
			result->handRank = FULL_HOUSE;
			return;
		}
	}

	if (result->nGroups == 3)
	{
		if (result->groups[0].count == 3 && result->groups[1].count == 1 && result->groups[2].count == 1
			|| result->groups[0].count == 1 && result->groups[1].count == 3 && result->groups[2].count == 1
			|| result->groups[0].count == 1 && result->groups[1].count == 1 && result->groups[2].count == 3)
		{
			result->handRank = THREE_OF_A_KIND;
			return;
		}

		if (result->groups[0].count == 2 && result->groups[1].count == 2 && result->groups[2].count == 1
			|| result->groups[0].count == 2 && result->groups[1].count == 1 && result->groups[2].count == 2
			|| result->groups[0].count == 1 && result->groups[1].count == 2 && result->groups[2].count == 2)
		{
			result->handRank = TWO_PAIR;
			return;
		}
	}

	if (result->nGroups == 4)
	{
		result->handRank = PAIR;
		return;
	}

	if (result->nConsecutiveRanks == 4 && result->nSameSuit == 4 && result->orderedCards[0].rank == TEN)
	{
		result->handRank = ROYAL_FLUSH;
		return;
	}
	else if ((result->nConsecutiveRanks == 4) && result->nSameSuit == 4)
	{
		result->handRank = STRAIGHT_FLUSH;
		return;
	}
	else if (result->nConsecutiveRanks == 4)
	{
		result->handRank = STRAIGHT;
		return;
	}
	else if (result->nSameSuit == 4)
	{
		result->handRank = FLUSH;
		return;
	}

	result->handRank = HIGH_CARD;
}

/*
	Compare to boards.

	[Returns]

		-1, if first board is ranked below second board.
		0, if both boards are ranked equal.
		1, if first board is ranked above second board.
*/
int Compare (board_t board1, board_t board2)
{
	HandRank (board1, &tempResult1);
	HandRank (board2, &tempResult2);

	if (tempResult1.handRank < tempResult2.handRank)
	{
		return -1;
	}

	if (tempResult1.handRank > tempResult2.handRank)
	{
		return 1;
	}

	if (tempResult1.handRank == HIGH_CARD || tempResult1.handRank == STRAIGHT || tempResult1.handRank == FLUSH || tempResult1.handRank == STRAIGHT_FLUSH)
	{
		return CompareSingleGroupCards (tempResult1.singleGroupCards, tempResult2.singleGroupCards, tempResult1.nSingleGroupCards);
	}

	if (tempResult1.handRank == PAIR)
	{
		if (tempResult1.lowPair->rank < tempResult2.lowPair->rank)
		{
			return -1;
		}

		if (tempResult1.lowPair->rank > tempResult2.lowPair->rank)
		{
			return 1;
		}

		return CompareSingleGroupCards (tempResult1.singleGroupCards, tempResult2.singleGroupCards, tempResult1.nSingleGroupCards);
	}

	if (tempResult1.handRank == TWO_PAIR)
	{
		if (tempResult1.highPair->rank < tempResult2.highPair->rank)
		{
			return -1;
		}

		if (tempResult1.highPair->rank > tempResult2.highPair->rank)
		{
			return 1;
		}

		if (tempResult1.lowPair->rank < tempResult2.lowPair->rank)
		{
			return -1;
		}

		if (tempResult1.lowPair->rank > tempResult2.lowPair->rank)
		{
			return 1;
		}

		return CompareSingleGroupCards (tempResult1.singleGroupCards, tempResult2.singleGroupCards, tempResult1.nSingleGroupCards);
	}

	if (tempResult1.handRank == THREE_OF_A_KIND)
	{
		if (tempResult1.trips->rank < tempResult2.trips->rank)
		{
			return -1;
		}

		if (tempResult1.trips->rank > tempResult2.trips->rank)
		{
			return 1;
		}

		// This can only happen with more than one deck.
		return CompareSingleGroupCards (tempResult1.singleGroupCards, tempResult2.singleGroupCards, tempResult1.nSingleGroupCards);
	}

	if (tempResult1.handRank == FULL_HOUSE)
	{
		if (tempResult1.trips->rank < tempResult2.trips->rank)
		{
			return -1;
		}

		if (tempResult1.trips->rank > tempResult2.trips->rank)
		{
			return 1;
		}

		if (tempResult1.lowPair->rank < tempResult2.lowPair->rank)
		{
			return -1;
		}

		if (tempResult1.lowPair->rank > tempResult2.lowPair->rank)
		{
			return 1;
		}

		return 0;
	}

	if (tempResult1.handRank == FOUR_OF_A_KIND)
	{
		if (tempResult1.four->rank < tempResult2.four->rank)
		{
			return -1;
		}

		if (tempResult1.four->rank > tempResult2.four->rank)
		{
			return 1;
		}

		// This can only happen with more than one deck.
		return CompareSingleGroupCards (tempResult1.singleGroupCards, tempResult2.singleGroupCards, tempResult1.nSingleGroupCards);
	}

	if (tempResult1.handRank == ROYAL_FLUSH)
	{
		return 0;
	}
}

bool Eval (eval_t* evalData)
{
	card_t deck[DECK_SIZE];
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
	for (rank_t r = TWO; r <= ACE; r++)
	{
		for (suit_t s = CLUBS; s <= SPADES; s++)
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
	int nPageEntries;
	board_t* boardsPage = (board_t*) malloc(PAGE_SIZE * sizeof (board_t));

	FSM_ResetRules (evalData->rules);

	//evalData->nCores

	if (combinationSize == 0)
	{
		nPageEntries = 1;
		evalData->nBoards += nPageEntries;
		memcpy (boardsPage[0], evalData->boardCards, sizeof (board_t));
		EvalPlayers (evalData, boardsPage, nPageEntries);
	}
	else
	{
		bool more = false;

		combination_info_t* info = E_Initialize (deck, nCards, combinationSize, PAGE_SIZE);

		do
		{
			more = E_Combinations (info);
			evalData->nBoards += info->nCombinations;
			nPageEntries = 0;

			if (evalData->nBoardCards == 0)
			{
//#ifdef DEBUG
//				D_WriteBoards ("C:\\Users\\Gabriel\\Desktop\\boards.txt", (board_t*) info->combinationBuffer, info->nCombinations);
//#endif
				EvalPlayers (evalData, (board_t*) info->combinationBuffer, info->nCombinations);
			}
			else
			{
				while (nPageEntries < info->nCombinations)
				{
					// Generate combination.
					for (int i = 0, j = 0; i < BOARD_SIZE; i++)
					{
						if (i < evalData->nBoardCards)
						{
							// Take board card if any.
							boardsPage[nPageEntries][i].rank = evalData->boardCards[i].rank;
							boardsPage[nPageEntries][i].suit = evalData->boardCards[i].suit;
						}
						else
						{
							// Take remaining deck card to complete combination.
							boardsPage[nPageEntries][i].rank = info->combinationBuffer[nPageEntries * combinationSize + j].rank;
							boardsPage[nPageEntries][i].suit = info->combinationBuffer[nPageEntries * combinationSize + j++].suit;
						}
					}

					nPageEntries++;
				}
//#ifdef DEBUG
//				D_WriteBoards ("C:\\Users\\Gabriel\\Desktop\\boards.txt", boardsPage, nPageEntries);
//#endif
				EvalPlayers (evalData, boardsPage, nPageEntries);
			}
		}
		while (more);

		E_Dispose (info);
	}

	for (int i = 0; i < evalData->nPlayers; i++)
	{
		int loses = evalData->nBoards - evalData->equities[i].wins - evalData->equities[i].ties;
		evalData->equities[i].winProbability = (double) evalData->equities[i].wins / (double) evalData->nBoards;
		evalData->equities[i].tieProbability = (double) evalData->equities[i].ties / (double) evalData->nBoards;
		evalData->equities[i].loseProbability = (double) loses / (double) evalData->nBoards;
	}

	if (boardsPage)
	{
		free (boardsPage);
	}

	return true;
}
