#ifndef POKER_H
#define POKER_H

#define DECK_SIZE 52
#define BOARD_SIZE 5
#define MAX_PLAYERS 10
#define MAX_CARDS 6

enum rules_t { HOLDEM, OMAHA, OMAHA5, OMAHA6 };

enum suit_t { NO_SUIT, CLUBS, DIAMONDS, HEARTS, SPADES };

enum rank_t { NO_RANK, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING, ACE };

enum hand_rank_t { HIGH_CARD = 1, PAIR, TWO_PAIR, THREE_OF_A_KIND, STRAIGHT, FLUSH, FULL_HOUSE, FOUR_OF_A_KIND, STRAIGHT_FLUSH, ROYAL_FLUSH };

struct card_t { enum rank_t rank; enum suit_t suit; };

#endif
