#include <string.h>
#include "cmbntn.h"
#include "fsm.h"

typedef void (*state_t)(combination_t combination);

int CardIndex;
int Indexes[2];
card_t HoleCards[6];
int CardsCount;
combination_t BoardCards;
state_t* CurrentStates;
int CurrentState;

void holdem_0(combination_t combination);
void holdem_init_1a(combination_t combination);
void holdem_1(combination_t combination);
void init_holdem_1b(combination_t combination);
void init_holdem_2(combination_t combination);
void holdem_2(combination_t combination);

void omaha_init(combination_t combination);
void omaha(combination_t combination);

state_t HoldemStates[] = { holdem_0, holdem_init_1a, holdem_1, init_holdem_1b, holdem_1, init_holdem_2, holdem_2 };

state_t OmahaStates[] = { omaha_init, omaha };

void holdem_0(combination_t combination)
{
	memcpy(combination, BoardCards, sizeof(combination_t));
	CurrentState++;
}

void holdem_init_1a(combination_t combination)
{
	CardIndex = 0;
	Indexes[0] = 0;
	CurrentState++;
}

void holdem_1(combination_t combination)
{
	combination[0].rank = HoleCards[CardsCount].rank;
	combination[0].suit = HoleCards[CardsCount].suit;

	for (int s = 0, d = 1; d < sizeof(combination_t); s++, d++)
	{
		if (s == Indexes[0])
		{
			continue;
		}

		combination[d].rank = BoardCards[s].rank;
		combination[d].suit = BoardCards[s].suit;
	}

	Indexes[0]++;

	if (Indexes[0] >= sizeof(combination_t))
	{
		CurrentState++;
	}
}

void init_holdem_1b(combination_t combination)
{
	CardIndex = 1;
	Indexes[0] = 0;
	CurrentState++;
}

void init_holdem_2(combination_t combination)
{
	CardIndex = 2;
	Indexes[0] = 0;
	Indexes[1] = 1;
	CurrentState++;
}

void holdem_2(combination_t combination)
{
	for (int i = 0; i < 2; i++)
	{
		combination[i].rank = BoardCards[Indexes[i]].rank;
		combination[i].suit = BoardCards[Indexes[i]].suit;
	}

	for (int s = 0, d = 2; d < sizeof(combination_t); s++, d++)
	{
		if (s == Indexes[0] || s == Indexes[1])
		{
			continue;
		}

		combination[d].rank = BoardCards[s].rank;
		combination[d].suit = BoardCards[s].suit;
	}

	if (next(Indexes, 2, sizeof(combination_t)))
	{
		CurrentState++;
	}
}

void omaha_init(combination_t combination)
{
}

void omaha(combination_t combination)
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

void FSM_reset_board_cards(combination_t board_cards)
{
	memcpy(BoardCards, board_cards, sizeof(combination_t));
	CurrentState = 0;
}

bool FSM_next(combination_t combination)
{
	if (CurrentState >= sizeof(CurrentStates))
	{
		return false;
	}

	CurrentStates[CurrentState](combination);

	return true;
}
