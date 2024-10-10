#include <string.h>
#include "poker.h"
#include "cmbntn.h"
#include "fsm.h"

#define HOLDEM_STATES 7
#define OMAHA_STATES 2

bool Holdem0 (struct card_t board[BOARD_SIZE], struct fsm_t* fsm)
{
	memcpy (board, fsm->boardCards, sizeof (struct card_t [BOARD_SIZE]));
	fsm->currentState++;
	return true;
}

bool HoldemInit1a (struct card_t board[BOARD_SIZE], struct fsm_t* fsm)
{
	fsm->cardIndex = 0;
	fsm->i0[0] = 0;
	fsm->currentState++;
	return false;
}

bool Holdem1 (struct card_t board[BOARD_SIZE], struct fsm_t* fsm)
{
	board[fsm->i0[0]].rank = fsm->holeCards[fsm->cardIndex].rank;
	board[fsm->i0[0]].suit = fsm->holeCards[fsm->cardIndex].suit;

	for (int i = 0; i < BOARD_SIZE; i++)
	{
		if (i == fsm->i0[0])
		{
			continue;
		}

		board[i].rank = fsm->boardCards[i].rank;
		board[i].suit = fsm->boardCards[i].suit;
	}

	fsm->i0[0]++;

	if (fsm->i0[0] == BOARD_SIZE)
	{
		fsm->currentState++;
	}

	return true;
}

bool InitHoldem1b (struct card_t board[BOARD_SIZE], struct fsm_t* fsm)
{
	fsm->cardIndex = 1;
	fsm->i0[0] = 0;
	fsm->currentState++;
	return false;
}

bool InitHoldem2 (struct card_t board[BOARD_SIZE], struct fsm_t* fsm)
{
	fsm->i0[0] = 0;
	fsm->i0[1] = 1;
	fsm->currentState++;
	return false;
}

bool Holdem2 (struct card_t board[BOARD_SIZE], struct fsm_t* fsm)
{
	for (int i = 0; i < 2; i++)
	{
		board[fsm->i0[i]].rank = fsm->holeCards[i].rank;
		board[fsm->i0[i]].suit = fsm->holeCards[i].suit;
	}

	for (int i = 0; i < BOARD_SIZE; i++)
	{
		if (i == fsm->i0[0] || i == fsm->i0[1])
		{
			continue;
		}

		board[i].rank = fsm->boardCards[i].rank;
		board[i].suit = fsm->boardCards[i].suit;
	}

	if (CMB_Next (fsm->i0, 2, BOARD_SIZE))
	{
		fsm->currentState++;
	}

	return true;
}

bool OmahaInit (struct card_t board[BOARD_SIZE], struct fsm_t* fsm)
{
	fsm->i0[0] = 0;
	fsm->i0[1] = 1;
	fsm->i1[0] = 0;
	fsm->i1[1] = 1;
	fsm->currentState++;
	return false;
}

bool Omaha (struct card_t board[BOARD_SIZE], struct fsm_t* fsm)
{
	for (int i = 0; i < 2; i++)
	{
		board[fsm->i1[i]].rank = fsm->holeCards[fsm->i0[i]].rank;
		board[fsm->i1[i]].suit = fsm->holeCards[fsm->i0[i]].suit;
	}

	for (int i = 0; i < BOARD_SIZE; i++)
	{
		if (i == fsm->i1[0] || i == fsm->i1[1])
		{
			continue;
		}

		board[i].rank = fsm->boardCards[i].rank;
		board[i].suit = fsm->boardCards[i].suit;
	}

	if (CMB_Next (fsm->i1, 2, BOARD_SIZE))
	{
		if (CMB_Next (fsm->i0, 2, fsm->nHoleCards))
		{
			fsm->currentState++;
		}		
		else
		{
			fsm->i1[0] = 0;
			fsm->i1[1] = 1;
		}
	}

	return true;
}

void FSM_Init (enum rules_t rules, struct fsm_t* fsm)
{
	if (rules == HOLDEM)
	{
		fsm->states[0] = Holdem0;
		fsm->states[1] = HoldemInit1a;
		fsm->states[2] = Holdem1;
		fsm->states[3] = InitHoldem1b;
		fsm->states[4] = Holdem1;
		fsm->states[5] = InitHoldem2;
		fsm->states[6] = Holdem2;
		fsm->nStates = HOLDEM_STATES;
	}
	else
	{
		fsm->states[0] = OmahaInit;
		fsm->states[1] = Omaha;
		fsm->nStates = OMAHA_STATES;
	}

	fsm->currentState = 0;
}

void FSM_ResetHoleCards (struct card_t* holeCards, int nHoleCards, struct fsm_t* fsm)
{
	memcpy (fsm->holeCards, holeCards, nHoleCards * sizeof (struct card_t));
	fsm->nHoleCards = nHoleCards;
	fsm->currentState = 0;
}

void FSM_ResetBoardCards (struct card_t boardCards[BOARD_SIZE], struct fsm_t* fsm)
{
	memcpy (fsm->boardCards, boardCards, sizeof (struct card_t [BOARD_SIZE]));
	fsm->currentState = 0;
}

bool FSM_Next (struct card_t board[BOARD_SIZE], struct fsm_t* fsm)
{
	if (fsm->currentState < 0 || fsm->currentState >= fsm->nStates)
	{
		return false;
	}

	bool done = false;

	do
	{
		done = fsm->states[fsm->currentState] (board, fsm);
	}
	while (!done && fsm->currentState < fsm->nStates);

	return true;
}
