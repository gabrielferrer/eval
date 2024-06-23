#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#ifdef DEBUG
#include <stdio.h>
#endif

#include "eval.h"
#include "enum.h"
#include "misc.h"
#include "fsm.h"

#define PAGE_SIZE 10000

typedef struct
{
	board_t best_board;
	int player_index;
} player_info_t;

board_t temp_board1;
board_t temp_board2;
hand_rank_result_t temp_result1;
hand_rank_result_t temp_result2;
player_info_t temp_player_info[MAX_PLAYERS];

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
		// For Omaha (4 hole cards) user can form 60 combinations total.
		// Two hole cards mandatory combined with 5 cards from board.
		// C(4, 2) * C(5, 2) = 6 * 10 = 60
		return 60;
	}

	if (rules == OMAHA5)
	{
		// For Omaha (5 hole cards) user can form 100 combinations total.
		// Two hole cards mandatory combined with 5 cards from board.
		// C(5, 2) * C(5, 2) = 10 * 10 = 100
		return 100;
	}

	if (rules == OMAHA6)
	{
		// For Omaha (6 hole cards) user can form 150 combinations total.
		// Two hole cards mandatory combined with 5 cards from board.
		// C(6, 2) * C(5, 2) = 15 * 10 = 150
		return 150;
	}

	return 0;
}

void calc_best_board(rules_t rules, board_t best_board)
{
	board_t current;

	FSM_next(current);
	// Initialize best board.
	memcpy(best_board, current, sizeof(board_t));

	while (FSM_next(current))
	{
		int comparison = compare(current, best_board);

		if (comparison <= 0)
		{
			continue;
		}

		memcpy(best_board, current, sizeof(board_t));
	}
}

void eval_players(eval_t* eval_data, board_t* boards_page, int page_entries)
{
	board_t best_board;

	for (int i = 0; i < page_entries; i++)
	{
		int best_count = 0;

		FSM_reset_board_cards(boards_page[i]);

		for (int j = 0; j < eval_data->players; j++)
		{
			FSM_reset_hole_cards(eval_data->hole_cards[j], eval_data->hole_cards_count);

			calc_best_board(eval_data->rules, best_board);

			if (best_count == 0)
			{
				memcpy(temp_player_info[best_count].best_board, best_board, sizeof(board_t));
				temp_player_info[best_count++].player_index = j;
				continue;
			}

			int comparison = compare(temp_player_info[0].best_board, best_board);

			if (comparison > 0)
			{
				continue;
			}

			if (comparison == 0)
			{
				memcpy(temp_player_info[best_count].best_board, best_board, sizeof(board_t));
				temp_player_info[best_count++].player_index = j;
				continue;
			}

			best_count = 0;
			memcpy(temp_player_info[best_count].best_board, best_board, sizeof(board_t));
			temp_player_info[best_count++].player_index = j;
		}

		if (best_count == 1)
		{
			eval_data->equities[temp_player_info[0].player_index].wins++;
		}
		else
		{
			for (int k = 0; k < best_count; k++)
			{
				eval_data->equities[temp_player_info[k].player_index].ties++;
			}
		}
	}
}

int compare_cards(const void* a, const void* b)
{
   card_t* c1 = (card_t*)a;
   card_t* c2 = (card_t*)b;

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
int compare_single_group_cards(card_t* c1[], card_t* c2[], int count)
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

void check_group(group_t* g, card_t* c, hand_rank_result_t* r)
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

bool straight_special_case(board_t b)
{
	return b[0].rank == TWO && b[1].rank == THREE && b[2].rank == FOUR && b[3].rank == FIVE && b[4].rank == ACE;
}

void reorder_straight_special_case(board_t b)
{
	suit_t s = b[BOARD_SIZE - 1].suit;

	for (int i = BOARD_SIZE - 1; i > 0; i--)
	{
		b[i].rank = b[i - 1].rank;
		b[i].suit = b[i - 1].suit;
	}

	b[0].rank = ONE;
	b[0].suit = s;
}

/*
	Calculate a given board's rank.

	[Returns]

		Struct with information about the hand rank.
*/
void hand_rank(board_t board, hand_rank_result_t* result)
{
	rank_t cr = NO_RANK;

	memset(result, 0, sizeof(hand_rank_result_t));
	result->group_count = -1;
	memcpy(result->ordered_cards, board, sizeof(board_t));
	qsort(result->ordered_cards, BOARD_SIZE, sizeof(card_t), compare_cards);

	int i = 0;

	while (i < BOARD_SIZE)
	{
		if (i + 1 < BOARD_SIZE)
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
	Compare to boards.

	[Returns]

		-1, if first board is ranked below second board.
		0, if both boards are ranked equal.
		1, if first board is ranked above second board.
*/
int compare(board_t board1, board_t board2)
{
	hand_rank(board1, &temp_result1);
	hand_rank(board2, &temp_result2);

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

bool eval(eval_t* eval_data)
{
	card_t deck[DECK_SIZE];
	eval_data->total_boards = 0;
	eval_data->errors = 0;

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		eval_data->equities[i].wins = 0;
		eval_data->equities[i].ties = 0;
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
	// It's important to traverse ranks first and then suits.
	// If not, INDEX macro will fail.
	for (rank_t r = TWO; r <= ACE; r++)
	{
		for (suit_t s = CLUBS; s <= SPADES; s++)
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
	}

	// Remove hole cards from deck.
	for (int i = 0; i < eval_data->players; i++)
	{
		for (int j = 0; j < eval_data->hole_cards_count; j++)
		{
			int k = INDEX(eval_data->hole_cards[i][j].rank, eval_data->hole_cards[i][j].suit);

			if (deck[k].rank == NO_RANK)
			{
				eval_data->errors |= DUPLICATED_CARD_FLAG;
				return false;
			}

			deck[k].rank = NO_RANK;
			deck[k].suit = NO_SUIT;
		}
	}

	int cards_count = 0;
	int source = cards_count + 1;

	// Compact remaining cards.
	while (source < DECK_SIZE)
	{	
		if (deck[cards_count].rank != NO_RANK)
		{
			++cards_count;
		}
		else if (deck[source].rank != NO_RANK)
		{
			deck[cards_count].rank = deck[source].rank;
			deck[cards_count++].suit = deck[source].suit;
			deck[source].rank = NO_RANK;
			deck[source].suit = NO_SUIT;
		}

		++source;
	}

	if (cards_count < BOARD_SIZE)
	{
		eval_data->errors |= INSUFFICIENT_COMBINATION_CARDS;
		return false;
	}

	int player_combinations = get_player_combinations(eval_data->rules);

	if (player_combinations == 0)
	{
		eval_data->errors |= INVALID_POKER_RULES;
		return false;
	}

	int combination_size = BOARD_SIZE - eval_data->board_cards_count;
	int page_entries;
	board_t* boards_page = (board_t*)malloc(PAGE_SIZE * sizeof(board_t));

	FSM_reset_rules(eval_data->rules);

	if (combination_size == 0)
	{
		page_entries = 1;
		eval_data->total_boards += page_entries;

		for (int i = 0; i < BOARD_SIZE; i++)
		{
			boards_page[0][i].rank = eval_data->board_cards[i].rank;
			boards_page[0][i].suit = eval_data->board_cards[i].suit;
		}

		eval_players(eval_data, boards_page, page_entries);
	}
	else
	{
		bool done = false;

		combination_info_t* info = initialize(deck, cards_count, combination_size, PAGE_SIZE);

		do
		{
			done = combinations(info);
			eval_data->total_boards += info->combination_count;
			page_entries = 0;

			if (eval_data->board_cards_count == 0)
			{
				eval_players(eval_data, (board_t*)info->combination_buffer, info->combination_count);
			}
			else
			{
				while (page_entries < info->combination_count)
				{
					// Generate combination.
					for (int i = 0; i < BOARD_SIZE; i++)
					{
						if (i < eval_data->board_cards_count)
						{
							// Take board card if any.
							boards_page[page_entries][i].rank = eval_data->board_cards[i].rank;
							boards_page[page_entries][i].suit = eval_data->board_cards[i].suit;
						}
						else
						{
							// Take remaining deck card to complete combination.
							boards_page[page_entries][i].rank = info->combination_buffer[page_entries * combination_size + i].rank;
							boards_page[page_entries][i].suit = info->combination_buffer[page_entries * combination_size + i].suit;
						}
					}

					page_entries++;
				}

				eval_players(eval_data, boards_page, page_entries);
			}
		}
		while (!done);

		dispose(info);
	}

	if (boards_page)
	{
		free(boards_page);
	}

	return true;
}
