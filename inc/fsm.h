#ifndef FSM_H
#define FSM_H

#include <stdbool.h>
#include "poker.h"

void FSM_reset_rules(rules_t rules);

void FSM_reset_hole_cards(card_t* hole_cards, int cards_count);

void FSM_reset_board_cards(combination_t board_cards);

bool FSM_next(combination_t combination);

#endif
