#include <string.h>
#include "cmbntn.h"
#include "fsm.h"

/*
	Fills a new board.

	[Returns]

		True if this state returns a valid board, false if next state must be
		called before returning a valid board.
*/
typedef bool (*state_t)(board_t board);

int CardIndex;
int Indexes[2];
card_t HoleCards[6];
int CardsCount;
board_t BoardCards;
state_t* States;
int StatesCount = 0;
int CurrentState = 0;
bool holdem_0(board_t board);
bool holdem_init_1a(board_t board);
bool holdem_1(board_t board);
bool init_holdem_1b(board_t board);
bool init_holdem_2(board_t board);
bool holdem_2(board_t board);

bool omaha_init(board_t board);
bool omaha(board_t board);

state_t HoldemStates[] = { holdem_0, holdem_init_1a, holdem_1, init_holdem_1b, holdem_1, init_holdem_2, holdem_2 };

state_t OmahaStates[] = { omaha_init, omaha };

bool holdem_0(board_t board)
{
	memcpy(board, BoardCards, sizeof(board_t));
	CurrentState++;
	return true;
}

bool holdem_init_1a(board_t board)
{
	CardIndex = 0;
	Indexes[0] = 0;
	CurrentState++;
	return false;
}

bool holdem_1(board_t board)
{
	board[0].rank = HoleCards[CardIndex].rank;
	board[0].suit = HoleCards[CardIndex].suit;

	for (int s = 0, d = 1; d < BOARD_SIZE; s++)
	{
		if (s == Indexes[0])
		{
			continue;
		}

		board[d].rank = BoardCards[s].rank;
		board[d++].suit = BoardCards[s].suit;
	}

	Indexes[0]++;

	if (Indexes[0] == BOARD_SIZE)
	{
		CurrentState++;
	}

	return true;
}

bool init_holdem_1b(board_t board)
{
	CardIndex = 1;
	Indexes[0] = 0;
	CurrentState++;
	return false;
}

bool init_holdem_2(board_t board)
{
	CardIndex = 2;
	Indexes[0] = 0;
	Indexes[1] = 1;
	CurrentState++;
	return false;
}

bool holdem_2(board_t board)
{
	for (int i = 0; i < 2; i++)
	{
		board[i].rank = BoardCards[Indexes[i]].rank;
		board[i].suit = BoardCards[Indexes[i]].suit;
	}

	for (int s = 0, d = 2; d < BOARD_SIZE; s++)
	{
		if (s == Indexes[0] || s == Indexes[1])
		{
			continue;
		}

		board[d].rank = BoardCards[s].rank;
		board[d++].suit = BoardCards[s].suit;
	}

	if (next(Indexes, 2, BOARD_SIZE))
	{
		CurrentState++;
	}

	return true;
}

bool omaha_init(board_t board)
{
	return true;
}

bool omaha(board_t board)
{
	return true;
}

void FSM_reset_rules(rules_t rules)
{
	if (rules == HOLDEM)
	{
		States = HoldemStates;
		StatesCount = sizeof(HoldemStates) / sizeof(state_t);
	} else
	{
		States = OmahaStates;
		StatesCount = sizeof(OmahaStates) / sizeof(state_t);
	}

	CurrentState = 0;
}

void FSM_reset_hole_cards(card_t* hole_cards, int cards_count)
{
	memcpy(HoleCards, hole_cards, cards_count * sizeof(card_t));
	CardsCount = cards_count;
	CurrentState = 0;
}

void FSM_reset_board_cards(board_t board_cards)
{
	memcpy(BoardCards, board_cards, sizeof(board_t));
	CurrentState = 0;
}

bool FSM_next(board_t board)
{
	if (CurrentState < 0 || CurrentState >= StatesCount)
	{
		return false;
	}

	bool done = false;

	do
	{
		done = States[CurrentState](board);
	}
	while (!done && CurrentState < StatesCount);

	return true;
}
