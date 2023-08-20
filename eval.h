#include <stdbool.h>

#define DECK_SIZE 52
#define COMBINATION_SIZE 5
#define MAX_HOLE_CARDS 6
#define MAX_PLAYERS 10
#define MAX_GROUPS 5

typedef enum { NO_SUIT, CLUBS, DIAMONDS, HEARTS, SPADES } suit_t;

typedef enum { NO_RANK, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING, ACE } rank_t;

typedef enum { HIGH_CARD = 1, PAIR, TWO_PAIR, THREE_OF_A_KIND, STRAIGHT, FLUSH, FULL_HOUSE, FOUR_OF_A_KIND, STRAIGHT_FLUSH, ROYAL_FLUSH } hand_rank_t;

typedef struct { rank_t rank; suit_t suit; } card_t;

typedef card_t combination_t[COMBINATION_SIZE];

typedef struct eval_t
{
	int players;
	int hole_cards_count;
	int dead_cards_count;
	card_t[MAX_PLAYERS][MAX_HOLE_CARDS] hole_cards;
	card_t[] dead_cards;
	double[MAX_PLAYERS] equities;
	bool duplicated_cards;
}

typedef struct { rank_t rank; int count; } group_t;

typedef struct
{
	group_t groups[MAX_GROUPS];                    // Groups of cards.
	int group_count;                               // How many groups of cards.
	int consecutive_ranks;                         // How many cards with consecutive ranks.
	int same_suit;                                 // How many cards with same suit.
	combination_t ordered_cards;                   // Ordered input cards.
	hand_rank_t hand_rank;                         // Hand rank found.
	group_t * low_pair;                            // Pointer into groups to low pair if hand rank is PAIR/TWO_PAIR/FULL_HOUSE.
	group_t * high_pair;                           // Pointer into groups to high pair if hand rank is TWO_PAIR.
	group_t * trips;                               // Pointer into groups to set of 3 cards if hand rank is THREE_OF_A_KIND/FULL_HOUSE.
	group_t * four;                                // Pointer into groups to set of 4 cards if hand rank is FOUR_OF_A_KIND.
	card_t * single_group_cards[COMBINATION_SIZE]; // Pointers to cards into ordered_cards that are single group.
	int single_group_cards_count;                  // How many cards with single group.
} hand_rank_result_t;

char * hand_rank_to_string(hand_rank_t hand_rank);

bool string_to_combination(char * combination_string, combination_t combination);

void hand_rank(combination_t combination, hand_rank_result_t * result);

int compare(combination_t combination1, combination_t combination2);

void eval(eval_t * eval_data, int count);
