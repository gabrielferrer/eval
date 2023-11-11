#include <stdlib.h>
#include <string.h>
#include "poker.h"
#include "misc.h"

rank_t to_rank(char r)
{
	if (r == '2') return TWO;
	if (r == '3') return THREE;
	if (r == '4') return FOUR;
	if (r == '5') return FIVE;
	if (r == '6') return SIX;
	if (r == '7') return SEVEN;
	if (r == '8') return EIGHT;
	if (r == '9') return NINE;
	if (r == 'T') return TEN;
	if (r == 'J') return JACK;
	if (r == 'Q') return QUEEN;
	if (r == 'K') return KING;
	if (r == 'A') return ACE;

	return NO_RANK;
}

suit_t to_suit(char s)
{
	if (s == 'c') return CLUBS;
	if (s == 'd') return DIAMONDS;
	if (s == 'h') return HEARTS;
	if (s == 's') return SPADES;

	return NO_SUIT;
}

char to_rank_string(rank_t r)
{
	if (r == TWO) return '2';
	if (r == THREE) return '3';
	if (r == FOUR) return '4';
	if (r == FIVE) return '5';
	if (r == SIX) return '6';
	if (r == SEVEN) return '7';
	if (r == EIGHT) return '8';
	if (r == NINE) return '9';
	if (r == TEN) return 'T';
	if (r == JACK) return 'J';
	if (r == QUEEN) return 'Q';
	if (r == KING) return 'K';
	if (r == ACE) return 'A';

	return '\0';
}

char to_suit_string(suit_t s)
{
	if (s == CLUBS) return 'c';
	if (s == DIAMONDS) return 'd';
	if (s == HEARTS) return 'h';
	if (s == SPADES) return 's';

	return '\0';	
}

char * hand_rank_to_string(hand_rank_t hand_rank)
{
	if (hand_rank == HIGH_CARD) return "High Card";
	if (hand_rank == PAIR) return "Pair";
	if (hand_rank == TWO_PAIR) return "Two Pair";
	if (hand_rank == THREE_OF_A_KIND) return "Three of a Kind";
	if (hand_rank == STRAIGHT) return "Straight";
	if (hand_rank == FLUSH) return "Flush";
	if (hand_rank == FULL_HOUSE) return "Full House";
	if (hand_rank == FOUR_OF_A_KIND) return "For of a Kind";
	if (hand_rank == STRAIGHT_FLUSH) return "Straight Flush";
	if (hand_rank == ROYAL_FLUSH) return "Royal Flush";

	return NULL;
}

bool string_to_combination(char * combination_string, combination_t combination)
{
	if (strlen(combination_string) != 2 * COMBINATION_SIZE)
	{
		return false;
	}

	rank_t r;
	suit_t s;

	for (int i = 0, j = 0; j < COMBINATION_SIZE; j++)
	{
		r = to_rank(combination_string[i++]);

		if (r == NO_RANK)
		{
			return false;
		}

		s = to_suit(combination_string[i++]);

		if (s == NO_SUIT)
		{
			return false;
		}

		combination[j].rank = r;
		combination[j].suit = s;
	}

	return true;
}

char * card_to_string(card_t * card, char * destination)
{
	destination[0] = to_rank_string(card->rank);
	destination[1] = to_suit_string(card->suit);
	destination[2] = '\0';

	return destination;
}
