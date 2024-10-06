#ifndef FSM_H
#define FSM_H

#include <stdbool.h>
#include "poker.h"

void FSM_ResetRules (rules_t rules);

void FSM_ResetHoleCards (card_t* holeCards, int nCards);

void FSM_ResetBoardCards (board_t boardCards);

bool FSM_Next (board_t board);

#endif
