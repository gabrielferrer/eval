#include <string.h>
#include <stdlib.h>
#include "thrdfnc.h"
#include "eval.h"
#include "cmbntn.h"
#include "fsm.h"

#define PAGE_SIZE 3000

struct player_info_t
{
	struct card_t bestBoard[BOARD_SIZE];
	int playerIndex;
};

struct thread_context_t
{
	int nPlayers;
	int nBoardCards;
	int nHoleCards;
	int nCards;
	int nCombinations;
	int nCombinationCards;
	int nPageEntries;
	enum rules_t rules;
	struct card_t boardCards[BOARD_SIZE];
	struct card_t holeCards[MAX_PLAYERS][MAX_CARDS];
	struct card_t cards[DECK_SIZE];
	int indexes[BOARD_SIZE];
	struct card_t (*boardsPage)[BOARD_SIZE];
	struct fsm_t fsm;
	struct thread_result_t results[MAX_PLAYERS];
	struct player_info_t playerInfo[MAX_PLAYERS];
};

bool StraightSpecialCase (struct card_t board[BOARD_SIZE])
{
	return board[0].rank == TWO && board[1].rank == THREE && board[2].rank == FOUR && board[3].rank == FIVE && board[4].rank == ACE;
}

void ReorderStraightSpecialCase (struct card_t board[BOARD_SIZE])
{
	enum suit_t s = board[BOARD_SIZE - 1].suit;

	for (int i = BOARD_SIZE - 1; i > 0; i--)
	{
		board[i].rank = board[i - 1].rank;
		board[i].suit = board[i - 1].suit;
	}

	board[0].rank = ONE;
	board[0].suit = s;
}

void CheckGroup (struct group_t* group, struct card_t* card, struct hand_rank_result_t* result)
{
	if (group->count == 1)
	{
		result->singleGroupCards[result->nSingleGroupCards++] = card;
	}
	else if (group->count == 2)
	{
		if (result->lowPair == NULL)
		{
			result->lowPair = group;
		}
		else
		{
			result->highPair = group;
		}
	}
	else if (group->count == 3)
	{
		result->trips = group;
	}
	else if (group->count == 4)
	{
		result->four = group;
	}
}

int CompareCards (const void* a, const void* b)
{
   struct card_t* c1 = (struct card_t*) a;
   struct card_t* c2 = (struct card_t*) b;

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
void HandRank (struct card_t board[BOARD_SIZE], struct hand_rank_result_t* result)
{
	enum rank_t cr = NO_RANK;

	memset (result, 0, sizeof (struct hand_rank_result_t));
	result->nGroups = -1;
	memcpy (result->orderedCards, board, sizeof (struct card_t [BOARD_SIZE]));
	qsort (result->orderedCards, BOARD_SIZE, sizeof (struct card_t), CompareCards);

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
	else if (result->nConsecutiveRanks == 4 && result->nSameSuit == 4)
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
int CompareSingleGroupCards (struct card_t* c1[], struct card_t* c2[], int count)
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
int Compare (struct card_t board1[BOARD_SIZE], struct card_t board2[BOARD_SIZE])
{
	struct hand_rank_result_t tempResult1;
	struct hand_rank_result_t tempResult2;	

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

void CalcBestBoard (struct thread_context_t* context, struct card_t bestBoard[BOARD_SIZE])
{
	struct card_t current[BOARD_SIZE];

	FSM_Next (current, &context->fsm);
	// Initialize best board.
	memcpy (bestBoard, current, sizeof (struct card_t [BOARD_SIZE]));

	while (FSM_Next (current, &context->fsm))
	{
		int comparison = Compare (current, bestBoard);

		if (comparison <= 0)
		{
			continue;
		}

		memcpy (bestBoard, current, sizeof (struct card_t [BOARD_SIZE]));
	}
}

void EvalPlayers (struct thread_context_t* context)
{
	struct card_t bestBoard[BOARD_SIZE];

	for (int i = 0; i < context->nPageEntries; i++)
	{
		int nBest = 0;

		FSM_ResetBoardCards (context->boardsPage[i], &context->fsm);

		for (int j = 0; j < context->nPlayers; j++)
		{
			FSM_ResetHoleCards (context->holeCards[j], context->nHoleCards, &context->fsm);

			CalcBestBoard (context, bestBoard);

			if (nBest == 0)
			{
				memcpy (context->playerInfo[nBest].bestBoard, bestBoard, sizeof (struct card_t [BOARD_SIZE]));
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
				memcpy (context->playerInfo[nBest].bestBoard, bestBoard, sizeof (struct card_t [BOARD_SIZE]));
				context->playerInfo[nBest++].playerIndex = j;
				continue;
			}
//#ifdef DEBUG
//				D_WriteSideBySideBoards ("C:\\Users\\Gabriel\\Desktop\\log.txt", bestBoard, context->playerInfo[0].bestBoard);
//#endif
			nBest = 0;
			memcpy (context->playerInfo[nBest].bestBoard, bestBoard, sizeof (struct card_t [BOARD_SIZE]));
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

void InitializeThreadContext (struct thread_context_t* context, struct thread_args_t* threadArgs)
{
	memset (context, 0, sizeof (struct thread_context_t));

	context->rules = threadArgs->rules;
	context->nPlayers = threadArgs->nPlayers;
	context->nBoardCards = threadArgs->nBoardCards;
	context->nHoleCards = threadArgs->nHoleCards;
	context->nCards = threadArgs->nCards;
	context->nCombinations = threadArgs->nCombinations;
	context->nCombinationCards = threadArgs->nCombinationCards;

	memcpy (context->boardCards, threadArgs->boardCards, threadArgs->nBoardCards * sizeof (struct card_t));

	for (int i = 0; i < threadArgs->nPlayers; i++)
	{
		memcpy (context->holeCards[i], threadArgs->holeCards[i], threadArgs->nHoleCards * sizeof (struct card_t));
	}

	memcpy (context->cards, threadArgs->cards, threadArgs->nCards * sizeof (struct card_t));
	memcpy (context->indexes, threadArgs->indexes, threadArgs->nCombinationCards * sizeof (int));

	context->boardsPage = (struct card_t (*)[BOARD_SIZE]) malloc ((threadArgs->nCombinationCards == 0 ? 1 : PAGE_SIZE) * sizeof (struct card_t [BOARD_SIZE]));

	FSM_Init (threadArgs->rules, &context->fsm);
}

void FreeThreadContext (struct thread_context_t* context)
{
	if (context->boardsPage) free (context->boardsPage);
}

THREAD_FUNC_RET_TYPE ThreadFunction (void* args)
{
	struct thread_args_t* threadArgs = (struct thread_args_t*) args;
	struct thread_context_t context;

	InitializeThreadContext (&context, threadArgs);

	if (context.nCombinationCards == 0)
	{
		context.nPageEntries = 1;
		memcpy (context.boardsPage[0], context.boardCards, sizeof (struct card_t [BOARD_SIZE]));
		EvalPlayers (&context);
	}
	else
	{
		bool done = false;

		do
		{
			context.nPageEntries = 0;

			do
			{
				for (int i = 0; i < BOARD_SIZE; i++)
				{
					if (i < context.nCombinationCards)
					{
						context.boardsPage[context.nPageEntries][i].rank = context.cards[context.indexes[i]].rank;
						context.boardsPage[context.nPageEntries][i].suit = context.cards[context.indexes[i]].suit;
					}
					else
					{
						context.boardsPage[context.nPageEntries][i].rank = context.boardCards[i - context.nCombinationCards].rank;
						context.boardsPage[context.nPageEntries][i].suit = context.boardCards[i - context.nCombinationCards].suit;
					}
				}

				++context.nPageEntries;
				// Adjust combination indexes for next combination.
				done = CMB_Next (context.indexes, context.nCombinationCards, context.nCards);
				--context.nCombinations;
			}
			while (!done && context.nPageEntries < PAGE_SIZE && context.nCombinations > 0);
//#ifdef DEBUG
//	D_WriteBoards ("C:\\Users\\Gabriel\\Desktop\\boards.txt", boardsPage, nPageEntries);
//#endif
			EvalPlayers (&context);
		}
		while (!done && context.nCombinations > 0);
	}

	for (int i = 0; i < context.nPlayers; i++)
	{
		threadArgs->results[i].wins = context.results[i].wins;
		threadArgs->results[i].ties = context.results[i].ties;
	}

	FreeThreadContext (&context);
}
