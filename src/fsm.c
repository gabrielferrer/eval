#include <string.h>
#include "cmbntn.h"
#include "fsm.h"

typedef void (*state_t)(board_t board);

int CardIndex;
int Indexes[2];
card_t HoleCards[6];
int CardsCount;
board_t BoardCards;
state_t* CurrentStates;
int CurrentState;

void holdem_0(board_t board);
void holdem_init_1a(board_t board);
void holdem_1(board_t board);
void init_holdem_1b(board_t board);
void init_holdem_2(board_t board);
void holdem_2(board_t board);

void omaha_init(board_t board);
void omaha(board_t board);

state_t HoldemStates[] = { holdem_0, holdem_init_1a, holdem_1, init_holdem_1b, holdem_1, init_holdem_2, holdem_2 };

state_t OmahaStates[] = { omaha_init, omaha };

void holdem_0(board_t board)
{
	memcpy(board, BoardCards, sizeof(board_t));
	CurrentState++;
}

void holdem_init_1a(board_t board)
{
	CardIndex = 0;
	Indexes[0] = 0;
	CurrentState++;
}

void holdem_1(board_t board)
{
	board[0].rank = HoleCards[CardsCount].rank;
	board[0].suit = HoleCards[CardsCount].suit;

	for (int s = 0, d = 1; d < sizeof(board_t); s++, d++)
	{
		if (s == Indexes[0])
		{
			continue;
		}

		board[d].rank = BoardCards[s].rank;
		board[d].suit = BoardCards[s].suit;
	}

	Indexes[0]++;

	if (Indexes[0] >= sizeof(board_t))
	{
		CurrentState++;
	}
}

void init_holdem_1b(board_t board)
{
	CardIndex = 1;
	Indexes[0] = 0;
	CurrentState++;
}

void init_holdem_2(board_t board)
{
	CardIndex = 2;
	Indexes[0] = 0;
	Indexes[1] = 1;
	CurrentState++;
}

void holdem_2(board_t board)
{
	for (int i = 0; i < 2; i++)
	{
		board[i].rank = BoardCards[Indexes[i]].rank;
		board[i].suit = BoardCards[Indexes[i]].suit;
	}

	for (int s = 0, d = 2; d < sizeof(board_t); s++, d++)
	{
		if (s == Indexes[0] || s == Indexes[1])
		{
			continue;
		}

		board[d].rank = BoardCards[s].rank;
		board[d].suit = BoardCards[s].suit;
	}

	if (next(Indexes, 2, sizeof(board_t)))
	{
		CurrentState++;
	}
}

void omaha_init(board_t board)
{
}

void omaha(board_t board)
{
}

void FSM_reset_rules(rules_t rules)
{
	CurrentStates = rules == HOLDEM ? HoldemStates : OmahaStates;
	CurrentState = 0;
}

void FSM_reset_hole_cards(card_t* hole_cards, int cards_count)
{
	memcpy(HoleCards, hole_cards, cards_count);
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
	if (CurrentState >= sizeof(CurrentStates))
	{
		return false;
	}

	CurrentStates[CurrentState](board);

	return true;
}
