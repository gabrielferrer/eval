#include <string.h>
#include "poker.h"
#include "cmbntn.h"
#include "fsm.h"

/*
	Fills a new board.

	[Returns]

		True if this state returns a valid board, false if next state must be
		called before returning a valid board.
*/
typedef bool (*state_t) (board_t board);

int CardIndex;
int I0[2];
int I1[2];
card_t HoleCards[MAX_CARDS];
int HoleCardsCount;
board_t BoardCards;
state_t* States;
int StatesCount = 0;
int CurrentState = 0;
bool Holdem0 (board_t board);
bool HoldemInit1a (board_t board);
bool Holdem1 (board_t board);
bool InitHoldem1b (board_t board);
bool InitHoldem2 (board_t board);
bool Holdem2 (board_t board);

bool OmahaInit (board_t board);
bool Omaha (board_t board);

state_t HoldemStates[] = { Holdem0, HoldemInit1a, Holdem1, InitHoldem1b, Holdem1, InitHoldem2, Holdem2 };

state_t OmahaStates[] = { OmahaInit, Omaha };

bool Holdem0 (board_t board)
{
	memcpy (board, BoardCards, sizeof (board_t));
	CurrentState++;
	return true;
}

bool HoldemInit1a (board_t board)
{
	CardIndex = 0;
	I0[0] = 0;
	CurrentState++;
	return false;
}

bool Holdem1 (board_t board)
{
	board[I0[0]].rank = HoleCards[CardIndex].rank;
	board[I0[0]].suit = HoleCards[CardIndex].suit;

	for (int i = 0; i < BOARD_SIZE; i++)
	{
		if (i == I0[0])
		{
			continue;
		}

		board[i].rank = BoardCards[i].rank;
		board[i].suit = BoardCards[i].suit;
	}

	I0[0]++;

	if (I0[0] == BOARD_SIZE)
	{
		CurrentState++;
	}

	return true;
}

bool InitHoldem1b (board_t board)
{
	CardIndex = 1;
	I0[0] = 0;
	CurrentState++;
	return false;
}

bool InitHoldem2 (board_t board)
{
	I0[0] = 0;
	I0[1] = 1;
	CurrentState++;
	return false;
}

bool Holdem2 (board_t board)
{
	for (int i = 0; i < 2; i++)
	{
		board[I0[i]].rank = HoleCards[i].rank;
		board[I0[i]].suit = HoleCards[i].suit;
	}

	for (int i = 0; i < BOARD_SIZE; i++)
	{
		if (i == I0[0] || i == I0[1])
		{
			continue;
		}

		board[i].rank = BoardCards[i].rank;
		board[i].suit = BoardCards[i].suit;
	}

	if (Next (I0, 2, BOARD_SIZE))
	{
		CurrentState++;
	}

	return true;
}

bool OmahaInit (board_t board)
{
	I0[0] = 0;
	I0[1] = 1;
	I1[0] = 0;
	I1[1] = 1;
	CurrentState++;
	return false;
}

bool Omaha (board_t board)
{
	for (int i = 0; i < 2; i++)
	{
		board[I1[i]].rank = HoleCards[I0[i]].rank;
		board[I1[i]].suit = HoleCards[I0[i]].suit;
	}

	for (int i = 0; i < BOARD_SIZE; i++)
	{
		if (i == I1[0] || i == I1[1])
		{
			continue;
		}

		board[i].rank = BoardCards[i].rank;
		board[i].suit = BoardCards[i].suit;
	}

	if (Next (I1, 2, BOARD_SIZE))
	{
		if (Next (I0, 2, HoleCardsCount))
		{
			CurrentState++;
		}		
		else
		{
			I1[0] = 0;
			I1[1] = 1;
		}
	}

	return true;
}

void FSM_ResetRules (rules_t rules)
{
	if (rules == HOLDEM)
	{
		States = HoldemStates;
		StatesCount = sizeof (HoldemStates) / sizeof (state_t);
	} else
	{
		States = OmahaStates;
		StatesCount = sizeof (OmahaStates) / sizeof (state_t);
	}

	CurrentState = 0;
}

void FSM_ResetHoleCards (card_t* holeCards, int nHoleCards)
{
	memcpy (HoleCards, holeCards, nHoleCards * sizeof (card_t));
	HoleCardsCount = nHoleCards;
	CurrentState = 0;
}

void FSM_ResetBoardCards (board_t boardCards)
{
	memcpy (BoardCards, boardCards, sizeof (board_t));
	CurrentState = 0;
}

bool FSM_Next (board_t board)
{
	if (CurrentState < 0 || CurrentState >= StatesCount)
	{
		return false;
	}

	bool done = false;

	do
	{
		done = States[CurrentState] (board);
	}
	while (!done && CurrentState < StatesCount);

	return true;
}
