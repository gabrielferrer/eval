#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "eval.h"

#define INDEX(r, s) (r << 2) + s - 8

combination_t temp_combination1;
combination_t temp_combination2;
hand_rank_result_t temp_result1;
hand_rank_result_t temp_result2;

int compare_cards(const void * a, const void * b)
{
   card_t * c1 = (card_t *)a;
   card_t * c2 = (card_t *)b;

   if (c1->rank < c2->rank)
   {
	   return -1;
   }

   if (c1->rank > c2->rank)
   {
	   return 1;
   }

   return 0;
}

/*
	Compare a set of cards.

	[Return]

		-1, if first card is ranked below second card.
		0, if both cards are ranked equal.
		1, if first card is ranked above second card.
*/
int compare_single_group_cards(card_t * c1[], card_t * c2[], int count)
{
	for (int i = count - 1; i >= 0; i--)
	{
		if (c1[i]->rank < c2[i]->rank)
		{
			return -1;
		}
		else if (c1[i]->rank > c2[i]->rank)
		{
			return 1;
		}
	}
	
	return 0;
}

void check_group(group_t * g, card_t * c, hand_rank_result_t * r)
{
	if (g->count == 1)
	{
		r->single_group_cards[r->single_group_cards_count++] = c;
	}
	else if (g->count == 2)
	{
		if (r->low_pair == NULL)
		{
			r->low_pair = g;
		}
		else
		{
			r->high_pair = g;
		}
	}
	else if (g->count == 3)
	{
		r->trips = g;
	}
	else if (g->count == 4)
	{
		r->four = g;
	}
}

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

bool straight_special_case(combination_t b)
{
	return b[0].rank == TWO && b[1].rank == THREE && b[2].rank == FOUR && b[3].rank == FIVE && b[4].rank == ACE;
}

void reorder_straight_special_case(combination_t b)
{
	suit_t s = b[COMBINATION_SIZE - 1].suit;

	for (int i = COMBINATION_SIZE - 1; i > 0; i--)
	{
		b[i].rank = b[i - 1].rank;
		b[i].suit = b[i - 1].suit;
	}

	b[0].rank = ONE;
	b[0].suit = s;
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

/*
	Calculate a given combination's rank.

	[Return]

		Struct with information about the hand rank.
*/
void hand_rank(combination_t combination, hand_rank_result_t * result)
{
	rank_t cr = NO_RANK;

	memset(result, 0, sizeof(hand_rank_result_t));
	result->group_count = -1;
	memcpy(result->ordered_cards, combination, sizeof(combination_t));
	qsort(result->ordered_cards, COMBINATION_SIZE, sizeof(card_t), compare_cards);

	int i = 0;

	while (i < COMBINATION_SIZE)
	{
		if (i + 1 < COMBINATION_SIZE)
		{
			// Check consecutive ranks.
			if (result->ordered_cards[i].rank + 1 == result->ordered_cards[i + 1].rank)
			{
				result->consecutive_ranks++;
			}

			// Check same suit.
			if (result->ordered_cards[i].suit == result->ordered_cards[i + 1].suit)
			{
				result->same_suit++;
			}
		}

		if (cr != result->ordered_cards[i].rank)
		{
			if (cr != NO_RANK)
			{
				check_group(&result->groups[result->group_count], &result->ordered_cards[i - 1], result);
			}

			// New group.
			// Increment group index and initialize group.
			result->group_count++;
			result->groups[result->group_count].rank = cr = result->ordered_cards[i].rank;
			result->groups[result->group_count].count = 1;
		}
		else
		{
			result->groups[result->group_count].count++;
		}

		i++;
	}

	// Check last group for extra information.
	check_group(&result->groups[result->group_count], &result->ordered_cards[i - 1], result);
	// Increment to get how many groups there are.
	result->group_count++;

	if (result->group_count == 2)
	{
		if (result->groups[0].count == 4 && result->groups[1].count == 1 || result->groups[0].count == 1 && result->groups[1].count == 4)
		{
			result->hand_rank = FOUR_OF_A_KIND;
			return;
		}

		if (result->groups[0].count == 3 && result->groups[1].count == 2 || result->groups[0].count == 2 && result->groups[1].count == 3)
		{
			result->hand_rank = FULL_HOUSE;
			return;
		}
	}

	if (result->group_count == 3)
	{
		if (result->groups[0].count == 3 && result->groups[1].count == 1 && result->groups[2].count == 1
			|| result->groups[0].count == 1 && result->groups[1].count == 3 && result->groups[2].count == 1
			|| result->groups[0].count == 1 && result->groups[1].count == 1 && result->groups[2].count == 3)
		{
			result->hand_rank = THREE_OF_A_KIND;
			return;
		}

		if (result->groups[0].count == 2 && result->groups[1].count == 2 && result->groups[2].count == 1
			|| result->groups[0].count == 2 && result->groups[1].count == 1 && result->groups[2].count == 2
			|| result->groups[0].count == 1 && result->groups[1].count == 2 && result->groups[2].count == 2)
		{
			result->hand_rank = TWO_PAIR;
			return;
		}
	}

	if (result->group_count == 4)
	{
		result->hand_rank = PAIR;
		return;
	}

	if (result->consecutive_ranks == 4 && result->same_suit == 4 && result->ordered_cards[0].rank == TEN)
	{
		result->hand_rank = ROYAL_FLUSH;
		return;
	}
	else if ((result->consecutive_ranks == 4) && result->same_suit == 4)
	{
		if (straight_special_case(result->ordered_cards))
		{
			reorder_straight_special_case(result->ordered_cards);
		}

		result->hand_rank = STRAIGHT_FLUSH;
		return;
	}
	else if (result->consecutive_ranks == 4)
	{
		if (straight_special_case(result->ordered_cards))
		{
			reorder_straight_special_case(result->ordered_cards);
		}

		result->hand_rank = STRAIGHT;
		return;
	}
	else if (result->same_suit == 4)
	{
		result->hand_rank = FLUSH;
		return;
	}

	result->hand_rank = HIGH_CARD;
}

/*
	Compare to combinations.

	[Return]

		-1, if first combination is ranked below second combination.
		0, if both combinations are ranked equal.
		1, if first combination is ranked above second combination.
*/
int compare(combination_t combination1, combination_t combination2)
{
	hand_rank(combination1, &temp_result1);
	hand_rank(combination2, &temp_result2);

	if (temp_result1.hand_rank < temp_result2.hand_rank)
	{
		return -1;
	}

	if (temp_result1.hand_rank > temp_result2.hand_rank)
	{
		return 1;
	}

	if (temp_result1.hand_rank == HIGH_CARD || temp_result1.hand_rank == STRAIGHT || temp_result1.hand_rank == FLUSH || temp_result1.hand_rank == STRAIGHT_FLUSH)
	{
		return compare_single_group_cards(temp_result1.single_group_cards, temp_result2.single_group_cards, temp_result1.single_group_cards_count);
	}

	if (temp_result1.hand_rank == PAIR)
	{
		if (temp_result1.low_pair->rank < temp_result2.low_pair->rank)
		{
			return -1;
		}

		if (temp_result1.low_pair->rank > temp_result2.low_pair->rank)
		{
			return 1;
		}

		return compare_single_group_cards(temp_result1.single_group_cards, temp_result2.single_group_cards, temp_result1.single_group_cards_count);
	}

	if (temp_result1.hand_rank == TWO_PAIR)
	{
		if (temp_result1.high_pair->rank < temp_result2.high_pair->rank)
		{
			return -1;
		}

		if (temp_result1.high_pair->rank > temp_result2.high_pair->rank)
		{
			return 1;
		}

		if (temp_result1.low_pair->rank < temp_result2.low_pair->rank)
		{
			return -1;
		}

		if (temp_result1.low_pair->rank > temp_result2.low_pair->rank)
		{
			return 1;
		}

		return compare_single_group_cards(temp_result1.single_group_cards, temp_result2.single_group_cards, temp_result1.single_group_cards_count);
	}

	if (temp_result1.hand_rank == THREE_OF_A_KIND)
	{
		if (temp_result1.trips->rank < temp_result2.trips->rank)
		{
			return -1;
		}

		if (temp_result1.trips->rank > temp_result2.trips->rank)
		{
			return 1;
		}

		// This can only happen with more than one deck.
		return compare_single_group_cards(temp_result1.single_group_cards, temp_result2.single_group_cards, temp_result1.single_group_cards_count);
	}

	if (temp_result1.hand_rank == FULL_HOUSE)
	{
		if (temp_result1.trips->rank < temp_result2.trips->rank)
		{
			return -1;
		}

		if (temp_result1.trips->rank > temp_result2.trips->rank)
		{
			return 1;
		}

		// This can only happen with more than one deck.
		return compare_single_group_cards(temp_result1.single_group_cards, temp_result2.single_group_cards, temp_result1.single_group_cards_count);
	}

	if (temp_result1.hand_rank == FOUR_OF_A_KIND)
	{
		if (temp_result1.four->rank < temp_result2.four->rank)
		{
			return -1;
		}

		if (temp_result1.four->rank > temp_result2.four->rank)
		{
			return 1;
		}

		// This can only happen with more than one deck.
		return compare_single_group_cards(temp_result1.single_group_cards, temp_result2.single_group_cards, temp_result1.single_group_cards_count);
	}

	if (temp_result1.hand_rank == ROYAL_FLUSH)
	{
		return 0;
	}
}

void eval(eval_t * eval_data, int count)
{
	card_t[DECK_SIZE] deck;
	int cards_count = DECK_SIZE;
	eval_data->duplicated_cards = false;

	// Initialize equities with invalid equities.
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		eval_data[i] = -1;
	}

	// Initialize deck.
	for (suit_t s = CLUBS; s <= SPADES; s++)
	{
		for (rank_t r = TWO; r <= ACE; r++)
		{
			int i = INDEX(r, s);
			deck[i].rank = r;
			deck[i].suit = s;
		}
	}

	// Remove dead cards from deck.
	for (int i = 0; i < eval_data->dead_cards_count; i++)
	{
		int j = INDEX(eval_data->dead_cards_count[i].rank, eval_data->dead_cards_count[i].suit);

		// Check for duplicated cards.
		if (deck[j].rank == NO_RANK)
		{
			eval_data->duplicated_cards = true;
			return;
		}

		deck[j].rank = NO_RANK;
		deck[j].suit = NO_SUIT;
		--cards_count;
	}

	// Remove hole cards from deck.
	for (int i = 0; i < eval_data->players; i++)
	{
		for (int j = 0; j < eval_data->hole_cards_count; j++)
		{
			int k = INDEX(eval_data->hole_cards[i][j].rank, eval_data->hole_cards[i][j].suit);

			if (deck[j].rank == NO_RANK)
			{
				eval_data->duplicated_cards = true;
				return;
			}

			deck[j].rank = NO_RANK;
			deck[j].suit = NO_SUIT;
			--cards_count;
		}
	}

	card_t * cards = (card_t *)malloc(cards_count * sizeof(card_t));

	// Compact remaining cards.
	for (int i = 0, j = 0; i < DECK_SIZE; i++)
	{
		if (deck[i].rank != NO_RANK)
		{
			cards[j].rank = deck[i].rank;
			cards[j++].suit = deck[i].suit;
		}
	}

	
	int k0 = 0, k1 = k0 + 1, k2 = k1 + 1, k3 = k2 + 1, k4 = k3 + 1;

	while()
	{
		
	}
}
