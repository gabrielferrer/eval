#ifndef MISC_H
#define MISC_H

#include <stdbool.h>
#include "poker.h"

enum rank_t ToRank (char r);

enum suit_t ToSuit (char s);

char ToRankString (enum rank_t r);

char ToSuitString (enum suit_t s);

char* HandRankToString (enum hand_rank_t handRank);

char* RulesToString (enum rules_t rules);

struct card_t* StringToCards (char* cardsString, struct card_t* cards);

char* CardToString (struct card_t* card, char* destination);

#endif
