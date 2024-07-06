#ifndef MISC_H
#define MISC_H

#include <stdbool.h>
#include "poker.h"

rank_t to_rank(char r);

suit_t to_suit(char s);

char to_rank_string(rank_t r);

char to_suit_string(suit_t s);

char* hand_rank_to_string(hand_rank_t hand_rank);

card_t* string_to_cards(char* cards_string, card_t* cards);

char* card_to_string(card_t* card, char* destination);

#endif
