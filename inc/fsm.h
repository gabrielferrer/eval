#ifndef FSM_H
#define FSM_H

#include <stdbool.h>
#include "poker.h"

#define MAX_STATES 7

struct fsm_t
{
	int cardIndex;
	int i0[2];
	int i1[2];
	struct card_t holeCards[MAX_CARDS];
	int nHoleCards;
	struct card_t boardCards[BOARD_SIZE];
	bool (*states[MAX_STATES]) (struct card_t board[BOARD_SIZE], struct fsm_t* fsm);
	int nStates;
	int currentState;
};

void FSM_Init (enum rules_t rules, struct fsm_t* fsm);

void FSM_ResetHoleCards (struct card_t* holeCards, int nHoleCards, struct fsm_t* fsm);

void FSM_ResetBoardCards (struct card_t boardCards[BOARD_SIZE], struct fsm_t* fsm);

bool FSM_Next (struct card_t board[BOARD_SIZE], struct fsm_t* fsm);

#endif
