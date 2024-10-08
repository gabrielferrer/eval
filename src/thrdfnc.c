#include <string.h>
#include <stdlib.h>
#include "thrdfnc.h"
#include "eval.h"
#include "cmbntn.h"
#include "fsm.h"

#define PAGE_SIZE 10000

typedef struct
{
	board_t bestBoard;
	int playerIndex;
} player_info_t;

typedef struct
{
	rules_t rules;
	int nPlayers;
	int nBoardCards;
	int nHoleCards;
	card_t* boardCards;
	card_t* holeCards[MAX_PLAYERS];
	card_t* cards;
	int nCards;
	int nCombinations;
	int nCombinationCards;
	int* indexes;
	board_t* boardsPage;
	int nCombinationBytes;
	card_t* combinationBuffer;
	card_t* bufferOffset;
	int nCombinationsInBuffer;
	card_t* currentCombination;
	int nPageEntries;
	thread_result_t results[MAX_PLAYERS];
	player_info_t playerInfo[MAX_PLAYERS];
} thread_context_t;

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

/*
	Compare to boards.

	[Returns]

		-1, if first board is ranked below second board.
		0, if both boards are ranked equal.
		1, if first board is ranked above second board.
*/
int Compare (board_t board1, board_t board2)
{
	hand_rank_result_t tempResult1;
	hand_rank_result_t tempResult2;	

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

void EvalPlayers (thread_context_t* context)
{
	board_t bestBoard;

	for (int i = 0; i < context->nPageEntries; i++)
	{
		int nBest = 0;

		FSM_ResetBoardCards (context->boardsPage[i]);

		for (int j = 0; j < context->nPlayers; j++)
		{
			FSM_ResetHoleCards (context->holeCards[j], context->nHoleCards);

			CalcBestBoard (context->rules, bestBoard);

			if (nBest == 0)
			{
				memcpy (context->playerInfo[nBest].bestBoard, bestBoard, sizeof (board_t));
				context->playerInfo[nBest++].playerIndex = j;
				continue;
			}

			int comparison = Compare (context->playerInfo[0].bestBoard, bestBoard);

			if (comparison > 0)
			{
//#ifdef DEBUG
//				D_WriteSideBySideBoards ("C:\\Users\\Gabriel\\Desktop\\log.txt", context->playerInfo[0].bestBoard, bestBoard);
//#endif
				continue;
			}

			if (comparison == 0)
			{
//#ifdef DEBUG
//				D_WriteSideBySideBoards ("C:\\Users\\Gabriel\\Desktop\\ties.txt", context->playerInfo[0].bestBoard, bestBoard);
//#endif
				memcpy (context->playerInfo[nBest].bestBoard, bestBoard, sizeof (board_t));
				context->playerInfo[nBest++].playerIndex = j;
				continue;
			}
//#ifdef DEBUG
//				D_WriteSideBySideBoards ("C:\\Users\\Gabriel\\Desktop\\log.txt", bestBoard, context->playerInfo[0].bestBoard);
//#endif
			nBest = 0;
			memcpy (context->playerInfo[nBest].bestBoard, bestBoard, sizeof (board_t));
			context->playerInfo[nBest++].playerIndex = j;
		}

		if (nBest == 1)
		{
			context->results[context->playerInfo[0].playerIndex].wins++;
		}
		else
		{
			for (int k = 0; k < nBest; k++)
			{
				context->results[context->playerInfo[k].playerIndex].ties++;
			}
		}
	}
}

void InitializeThreadContext (thread_context_t* context, thread_args_t* threadArgs)
{
	context->rules = threadArgs->rules;
	context->nPlayers = threadArgs->nPlayers;
	context->nBoardCards = threadArgs->nBoardCards;
	context->nHoleCards = threadArgs->nHoleCards;
	context->boardCards = (card_t*) malloc (threadArgs->nBoardCards * sizeof (card_t));
	memcpy (context->boardCards, threadArgs->boardCards, threadArgs->nBoardCards * sizeof (card_t));

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		context->holeCards[i] = (card_t*) malloc (threadArgs->nHoleCards * sizeof (card_t));
		memcpy (context->holeCards[i], threadArgs->holeCards[i], threadArgs->nHoleCards * sizeof (card_t));
	}

	context->cards = (card_t*) malloc (threadArgs->nCards * sizeof (card_t));
	memcpy (context->cards, threadArgs->cards, threadArgs->nCards * sizeof (card_t));
	context->nCards = threadArgs->nCards;
	context->nCombinations = threadArgs->nCombinations;
	context->nCombinationCards = threadArgs->nCombinationCards;
	context->indexes = (int*) malloc (threadArgs->nCombinationCards * sizeof (int));
	context->boardsPage = (board_t*) malloc (PAGE_SIZE * sizeof (board_t));
	context->nCombinationBytes = threadArgs->nCombinationCards * sizeof (card_t);
	context->combinationBuffer = (card_t*) malloc (threadArgs->nCombinationCards * PAGE_SIZE * sizeof (card_t));
	context->bufferOffset = context->combinationBuffer;
	context->nCombinationsInBuffer = 0;
	context->currentCombination = (card_t*) malloc (threadArgs->nCombinationCards * sizeof (card_t));
	context->nPageEntries = 0;
}

void FreeThreadContext (thread_context_t* context)
{
	free (context->boardCards);

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		free (context->holeCards[i]);
	}

	free (context->cards);
	free (context->indexes);
	free (context->boardsPage);
	free (context->combinationBuffer);
	free (context->currentCombination);
}

THREAD_FUNC_RET_TYPE ThreadFunction (void* args)
{
	thread_args_t* threadArgs = (thread_args_t*) args;
	thread_context_t context;

	InitializeThreadContext (&context, threadArgs);

	bool done = false;

	do
	{
		context.nCombinationsInBuffer = 0;

		do
		{
			// Prepare current combination.
			for (int i = 0; i < context.nCombinationCards; i++)
			{
				context.currentCombination[i].rank = context.cards[context.indexes[i]].rank;
				context.currentCombination[i].suit = context.cards[context.indexes[i]].suit;
			}

			// Copy combination to buffer.
			memcpy (context.bufferOffset, context.currentCombination, context.nCombinationBytes);
			// Adjust destination buffer index.
			context.bufferOffset += context.nCombinationCards;
			context.nCombinationsInBuffer++;
			// Adjust combination indexes for next combination.
			done = CMB_Next (context.indexes, context.nCombinationCards, context.nCards);
		}
		while (!done && context.nCombinationsInBuffer < PAGE_SIZE);

		context.nPageEntries = 0;

		if (context.nBoardCards == 0)
		{
//#ifdef DEBUG
//	D_WriteBoards ("C:\\Users\\Gabriel\\Desktop\\boards.txt", (board_t*) context.combinationBuffer, context.nCombinationsInBuffer);
//#endif
			EvalPlayers (&context);
		}
		else
		{
			while (context.nPageEntries < context.nCombinationsInBuffer)
			{
				// Generate combination.
				for (int i = 0, j = 0; i < BOARD_SIZE; i++)
				{
					if (i < context.nBoardCards)
					{
						// Take board card if any.
						context.boardsPage[context.nPageEntries][i].rank = context.boardCards[i].rank;
						context.boardsPage[context.nPageEntries][i].suit = context.boardCards[i].suit;
					}
					else
					{
						// Take remaining deck card to complete combination.
						context.boardsPage[context.nPageEntries][i].rank = context.combinationBuffer[context.nPageEntries * context.nCombinationCards + j].rank;
						context.boardsPage[context.nPageEntries][i].suit = context.combinationBuffer[context.nPageEntries * context.nCombinationCards + j++].suit;
					}
				}

				context.nPageEntries++;
			}
//#ifdef DEBUG
//	D_WriteBoards ("C:\\Users\\Gabriel\\Desktop\\boards.txt", boardsPage, nPageEntries);
//#endif
			EvalPlayers (&context);
		}
	}
	while (!done);

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		threadArgs->results[i].wins = context.results[i].wins;
		threadArgs->results[i].ties = context.results[i].ties;
	}

	FreeThreadContext (&context);
}
