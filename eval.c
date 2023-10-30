#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "poker.h"
#include "enum.h"

#define PAGE_SIZE 1000
#define INDEX(r, s) (r << 2) + s - 8

typedef struct
{
	int combinations_count;
	combination_t * combinations;
	int wins;
	int ties;
} player_info_t;

combination_t temp_combination1;
combination_t temp_combination2;
hand_rank_result_t temp_result1;
hand_rank_result_t temp_result2;

bool adjust_indexes_with_5(int x[], int c)
{
	if (x[4] + 1 < c)
	{
		x[4]++;
	}
	else if (x[3] + 1 < x[4])
	{
		x[3]++;

		if (x[3] + 1 < x[4])
		{
			x[4] = x[3] + 1;
		}
	}
	else if (x[2] + 1 < x[3])
	{
		x[2]++;

		if (x[2] + 1 < x[3])
		{
			x[3] = x[2] + 1;
			x[4] = x[3] + 1;
		}
	}
	else if (x[1] + 1 < x[2])
	{
		x[1]++;

		if (x[1] + 1 < x[2])
		{
			x[2] = x[1] + 1;
			x[3] = x[2] + 1;
			x[4] = x[3] + 1;
		}
	}
	else if (x[0] + 1 < x[1])
	{
		x[0]++;

		if (x[0] + 1 < x[1])
		{
			x[1] = x[0] + 1;
			x[2] = x[1] + 1;
			x[3] = x[2] + 1;
			x[4] = x[3] + 1;
		}
	}
	else
	{
		return true;
	}

	return false;
}

bool adjust_indexes_with_4(int x[], int c)
{
	if (x[3] + 1 < c)
	{
		x[3]++;
	}
	else if (x[2] + 1 < x[3])
	{
		x[2]++;

		if (x[2] + 1 < x[3])
		{
			x[3] = x[2] + 1;
		}
	}
	else if (x[1] + 1 < x[2])
	{
		x[1]++;

		if (x[1] + 1 < x[2])
		{
			x[2] = x[1] + 1;
			x[3] = x[2] + 1;
		}
	}
	else if (x[0] + 1 < x[1])
	{
		x[0]++;

		if (x[0] + 1 < x[1])
		{
			x[1] = x[0] + 1;
			x[2] = x[1] + 1;
			x[3] = x[2] + 1;
		}
	}
	else
	{
		return true;
	}

	return false;
}

bool adjust_indexes_with_3(int x[], int c)
{
	if (x[2] + 1 < c)
	{
		x[2]++;
	}
	else if (x[1] + 1 < x[2])
	{
		x[1]++;

		if (x[1] + 1 < x[2])
		{
			x[2] = x[1] + 1;
		}
	}
	else if (x[0] + 1 < x[1])
	{
		x[0]++;

		if (x[0] + 1 < x[1])
		{
			x[1] = x[0] + 1;
			x[2] = x[1] + 1;
		}
	}
	else
	{
		return true;
	}

	return false;
}

bool adjust_indexes_with_2(int x[], int c)
{
	if (x[1] + 1 < c)
	{
		x[1]++;
	}
	else if (x[0] + 1 < x[1])
	{
		x[0]++;

		if (x[0] + 1 < x[1])
		{
			x[1] = x[0] + 1;
		}
	}
	else
	{
		return true;
	}

	return false;
}

bool adjust_indexes_with_1(int x[], int c)
{
	if (x[0] + 1 < c)
	{
		x[0]++;
	}
	else
	{
		return true;
	}

	return false;
}

/*
	Returns how many combinations there are for the given poker rules.

	[Returns]

		The number of combinations.
*/
int get_player_combinations(rules_t rules)
{
	if (rules == HOLDEM)
	{
		// For Hold'em user can form 21 combinations total.
		// Zero, one or two hole cards can be used combined with 5 cards from board.
		// C(5, 0) + 2 * C(5, 1) + C(5, 2) = 1 + 2 * 5 + 10 = 21
		return 21;
	}

	if (rules == OMAHA)
	{
		// For Omaha (4 hole cards) user can form combinations total.
		// Two hole cards mandatory combined with 5 cards from board.
		// C(4, 2) * C(5, 2) = 6 * 10 = 60
		return 60;
	}

	if (rules == OMAHA5)
	{
		// For Omaha (5 hole cards) user can form combinations total.
		// Two hole cards mandatory combined with 5 cards from board.
		// C(5, 2) * C(5, 2) = 10 * 10 = 100
		return 100;
	}

	if (rules == OMAHA6)
	{
		// For Omaha (6 hole cards) user can form combinations total.
		// Two hole cards mandatory combined with 5 cards from board.
		// C(6, 2) * C(5, 2) = 15 * 10 = 150
		return 150;
	}

	return 0;
}

//void combine(card_t * source, card_t * destination, card_t * replacement, int replacement_count)
//{	
//}

void generate_player_combinations(rules_t rules, combination_t board_cards, card_t * hole_cards, player_info_t * player_info)
{
	//int i = 0;

	//if (rules == HOLDEM)
	//{
		// C(5, 0)
	//	memcpy(player_info->combinations[i++], board_cards, sizeof(combination_t));
		// 2 * C(5, 1)
		//hole_cards[0]
		//hole_cards[1]

	//	return;
	//}
}

void eval_with_players(eval_t * eval_data, player_info_t * players_info, combination_t * combinations_page, int page_entries)
{
	for (int i = 0; i < page_entries; i++)
	{
		for (int j = 0; j < eval_data->players; j++)
		{
			generate_player_combinations(eval_data->rules, combinations_page[i], eval_data->hole_cards[j], &players_info[j]);
		}

		//compare(combinations_page[i], );
	}
}

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

	[Returns]

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

	[Returns]

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

	[Returns]

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

bool eval(eval_t * eval_data)
{
	combination_t combinations_page[PAGE_SIZE];
	card_t deck[DECK_SIZE];
	player_info_t * players_info;
	int cards_count = DECK_SIZE;
	eval_data->errors = 0;

	// Initialize equities with invalid equities.
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		eval_data->equities[i].win_percent = -1;
		eval_data->equities[i].lose_percent = -1;
		eval_data->equities[i].tie_percent = -1;
	}

	if (eval_data->board_cards_count < 0 || eval_data->board_cards_count == 1 || eval_data->board_cards_count == 2 || eval_data->board_cards_count > 5)
	{
		eval_data->errors |= INVALID_BOARD_CARDS_COUNT;
		return false;
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

	for (int i = 0; i < eval_data->board_cards_count; i++)
	{
		int j = INDEX(eval_data->board_cards[i].rank, eval_data->board_cards[i].suit);

		// Check for duplicated cards.
		if (deck[j].rank == NO_RANK)
		{
			eval_data->errors |= DUPLICATED_CARD_FLAG;
			return false;
		}

		deck[j].rank = NO_RANK;
		deck[j].suit = NO_SUIT;
		--cards_count;
	}

	// Remove dead cards from deck.
	for (int i = 0; i < eval_data->dead_cards_count; i++)
	{
		int j = INDEX(eval_data->dead_cards[i].rank, eval_data->dead_cards[i].suit);

		if (deck[j].rank == NO_RANK)
		{
			eval_data->errors |= DUPLICATED_CARD_FLAG;
			return false;
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
				eval_data->errors |= DUPLICATED_CARD_FLAG;
				return false;
			}

			deck[j].rank = NO_RANK;
			deck[j].suit = NO_SUIT;
			--cards_count;
		}
	}

	if (cards_count < COMBINATION_SIZE)
	{
		eval_data->errors |= INSUFFICIENT_COMBINATION_CARDS;
		return false;
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

	int combinations = get_player_combinations(eval_data->rules);

	if (combinations == 0)
	{
		eval_data->errors |= INVALID_POKER_RULES;
		return false;
	}

	players_info = (player_info_t *)malloc(eval_data->players * sizeof(player_info_t));

	for (int i = 0; i < eval_data->players; i++)
	{
		players_info[i].combinations = (combination_t *)malloc(combinations * sizeof(combination_t));
	}

	int combination_indexes_count = COMBINATION_SIZE - eval_data->board_cards_count;
	int page_entries;

	if (combination_indexes_count == 0)
	{
		page_entries = 1;

		for (int i = 0; i < COMBINATION_SIZE; i++)
		{
			combinations_page[0][i].rank = eval_data->board_cards[i].rank;
			combinations_page[0][i].suit = eval_data->board_cards[i].suit;
		}

		eval_with_players(eval_data, players_info, combinations_page, page_entries);
	}
	else
	{
		int combination_indexes[COMBINATION_SIZE];
		bool (*adjust_indexes)(int [], int) = combination_indexes_count == 5 ? &adjust_indexes_with_5
			: combination_indexes_count == 4 ? &adjust_indexes_with_4
			: combination_indexes_count == 3 ? &adjust_indexes_with_3
			: combination_indexes_count == 2 ? &adjust_indexes_with_2
			: combination_indexes_count == 1 ? &adjust_indexes_with_1
			: NULL;
		bool done = false;

		// Initialize indexes to cards.
		for (int i = 0; i < COMBINATION_SIZE; i++)
		{
			if (i < combination_indexes_count)
			{
				combination_indexes[i] = i;
			}
			else
			{
				combination_indexes[i] = -1;
			}
		}

		do
		{
			page_entries = 0;

			while (page_entries < PAGE_SIZE && !done)
			{
				// Generate combination.
				for (int i = 0; i < COMBINATION_SIZE; i++)
				{
					if (i < eval_data->board_cards_count)
					{
						// Take board card if any.
						combinations_page[page_entries][i].rank = eval_data->board_cards[i].rank;
						combinations_page[page_entries][i].suit = eval_data->board_cards[i].suit;
					}
					else
					{
						// Take remaining deck card to complete combination.
						combinations_page[page_entries][i].rank = cards[combination_indexes[i]].rank;
						combinations_page[page_entries][i].suit = cards[combination_indexes[i]].suit;
					}
				}

				page_entries++;

				done = adjust_indexes(combination_indexes, cards_count);
			}

			eval_with_players(eval_data, players_info, combinations_page, page_entries);
		}
		while (!done);
	}

	if (cards)
	{
		free(cards);
	}
	
	if (players_info)
	{
		for (int i = 0; i < eval_data->players; i++)
		{
			if (players_info[i].combinations)
			{
				free(players_info[i].combinations);
			}
		}

		free(players_info);
	}

	return true;
}
