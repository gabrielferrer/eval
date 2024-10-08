#ifndef MISC_H
#define MISC_H

#include <stdbool.h>
#include "poker.h"

rank_t ToRank (char r);

suit_t ToSuit (char s);

char ToRankString (rank_t r);

char ToSuitString (suit_t s);

char* HandRankToString (hand_rank_t handRank);

card_t* StringToCards (char* cardsString, card_t* cards);

char* CardToString (card_t* card, char* destination);

#endif
