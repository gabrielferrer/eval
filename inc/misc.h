#ifndef MISC_H
#define MISC_H

#include <stdbool.h>

rank_t to_rank(char r);

suit_t to_suit(char s);

char to_rank_string(rank_t r);

char to_suit_string(suit_t s);

char* hand_rank_to_string(hand_rank_t hand_rank);

bool string_to_board(char* board_string, board_t board);

char* card_to_string(card_t* card, char* destination);

#endif
