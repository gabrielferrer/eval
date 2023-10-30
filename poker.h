#ifndef POKER_H
#define POKER_H

typedef enum { HOLDEM, OMAHA, OMAHA5, OMAHA6 } rules_t;

typedef enum { NO_SUIT, CLUBS, DIAMONDS, HEARTS, SPADES } suit_t;

typedef enum { NO_RANK, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING, ACE } rank_t;

typedef enum { HIGH_CARD = 1, PAIR, TWO_PAIR, THREE_OF_A_KIND, STRAIGHT, FLUSH, FULL_HOUSE, FOUR_OF_A_KIND, STRAIGHT_FLUSH, ROYAL_FLUSH } hand_rank_t;

typedef struct { rank_t rank; suit_t suit; } card_t;

#endif
