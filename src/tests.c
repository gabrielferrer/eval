#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "enum.h"
#include "eval.h"
#include "misc.h"
#include "fsm.h"

#define BUFFER_SIZE 10000

typedef struct { double e_win; double e_lose; double e_tie; } eq_t;

hand_rank_result_t r;

void hand_rank_test(char* board_string, hand_rank_t hr)
{
	board_t board;
	hand_rank_result_t result;

	if (string_to_cards(board_string, board) == NULL)
	{
		printf("Invalid input: %s", board_string);
		return;
	}

	hand_rank(board, &result);

	printf("Board: %s. Expected: %s. Actual: %s\n", board_string, hand_rank_to_string(hr), hand_rank_to_string(result.hand_rank));
}

void compare_test(char* board_string1, char* board_string2, int e)
{
	board_t board1;
	board_t board2;
	hand_rank_result_t result1;
	hand_rank_result_t result2;

	if (string_to_cards(board_string1, board1) == NULL)
	{
		printf("Invalid input: %s", board_string1);
		return;
	}

	if (string_to_cards(board_string2, board2) == NULL)
	{
		printf("Invalid input: %s", board_string2);
		return;
	}

	printf("1st. board: %s. 2nd. board: %s. Expected: %d. Actual: %d\n", board_string1, board_string2, e, compare(board1, board2));
}

void eval_test(rules_t rules, int players, char* board_cards, char* dead_cards, ...)
{
	va_list valist;
	eval_t eval_data;
	board_t board;
	card_t dead[DECK_SIZE];
	card_t hole[MAX_PLAYERS * 6];
	eq_t equities[MAX_PLAYERS];
	char card_buffer[3];

	eval_data.rules = rules;
	eval_data.players = players;
	eval_data.board_cards = string_to_cards(board_cards, board);
	eval_data.dead_cards = string_to_cards(dead_cards, dead);

	eval_data.board_cards_count = board_cards != NULL ? strlen(board_cards) / 2 : 0;
	eval_data.dead_cards_count = dead_cards != NULL ? strlen(dead_cards) / 2 : 0;

	va_start(valist, dead_cards);

	eval_data.hole_cards_count = 0;

	for (int i = 0; i < players; i++)
	{
		char* hole_cards = va_arg(valist, char*);
		eval_data.hole_cards[i]	= &hole[i * 6];
		string_to_cards(hole_cards, eval_data.hole_cards[i]);

		if (hole_cards != NULL && eval_data.hole_cards_count == 0)
		{
			eval_data.hole_cards_count = strlen(hole_cards) / 2;
		}
	}

	for (int i = 0; i < players; i++)
	{
		equities[i].e_win = va_arg(valist, double);
		equities[i].e_lose = va_arg(valist, double);
		equities[i].e_tie = va_arg(valist, double);
	}

	va_end(valist);

	eval(&eval_data);

	if (eval_data.errors != 0)
	{
		if (eval_data.errors & DUPLICATED_CARD_FLAG)
		{
			printf ("Duplicated card.\n");
		}

		if (eval_data.errors & INSUFFICIENT_COMBINATION_CARDS)
		{
			printf ("Insufficient combination cards.\n");
		}

		if (eval_data.errors & INVALID_BOARD_CARDS_COUNT)
		{
			printf ("Invalid board cards count.\n");
		}

		if (eval_data.errors & INVALID_POKER_RULES)
		{
			printf ("Invalid poker rules.\n");
		}

		return;
	}

	for (int i = 0; i < players; i++)
	{
		int loses = eval_data.total_boards - eval_data.equities[i].wins - eval_data.equities[i].ties;
		printf("Data for player %i (", i);

		for (int j = 0; j < eval_data.hole_cards_count; j++)
		{
			card_to_string(&eval_data.hole_cards[i][j], card_buffer);
			printf("%s", card_buffer);
		}

		printf(")\n");
		printf("\tWins: %i, loses: %i, ties: %i\n", eval_data.equities[i].wins, loses, eval_data.equities[i].ties);
		printf("\tEq. win:  (%f, %f)\n", equities[i].e_win, eval_data.equities[i].win_probability * 100.0d);
		printf("\tEq. lose: (%f, %f)\n", equities[i].e_lose, eval_data.equities[i].lose_probability * 100.0d);
		printf("\tEq. tie:  (%f, %f)\n", equities[i].e_tie, eval_data.equities[i].tie_probability * 100.0d);
		printf("\n");
	}
}

void rank_tests()
{
	hand_rank_test("9c4cKd2h6s", HIGH_CARD);
	hand_rank_test("9c4cKdKh6s", PAIR);
	hand_rank_test("Kh9c4cKd4s", TWO_PAIR);
	hand_rank_test("8h4c4hKd4s", THREE_OF_A_KIND);
	hand_rank_test("8h4c7h5d6s", STRAIGHT);
	hand_rank_test("5s2cAh4d3d", STRAIGHT);
	hand_rank_test("3hAhTh5h8h", FLUSH);
	hand_rank_test("3hTsTh3cTd", FULL_HOUSE);
	hand_rank_test("KhKsKcJcKd", FOUR_OF_A_KIND);
	hand_rank_test("7c4c3c6c5c", STRAIGHT_FLUSH);
	hand_rank_test("JdAdQdKdTd", ROYAL_FLUSH);
}

void comparation_tests()
{
	// High card.
	compare_test("7dKsAc2cJs", "JcAs7dKs6h", -1);
	compare_test("8dKsAc2cJs", "JcAs7dKs6h", 1);
	compare_test("8dKsAc2cQs", "JcAs7dKs6h", 1);
	compare_test("7dTsAc2c9s", "JcAs8d9s6h", -1);
	compare_test("7dTsAc9c8s", "TcAs8d9s7h", 0);

	// Pair
	compare_test("AhAd5cTd6s", "AcAs7d8s3h", 1);
	compare_test("AhAd5cJd6s", "AcAs7dJs3h", -1);
	compare_test("AhAd2cJd7s", "AcAs7dJs6h", -1);
	compare_test("5h4dQcJdQs", "5c4sQdJsQh", 0);
	compare_test("Th2dTc7dAs", "2c4sQd6s2h", 1);

	// Two pair
	compare_test("AhAd5cTdTs", "AcAsTc8sTh", -1);
	compare_test("AhAd8cTdTs", "AcAsTc8sTh", 0);
	compare_test("2h5d2cJdJs", "Jc6s7dJh6h", -1);

	// Three of a kind
	compare_test("AhAd5cAsTs", "QcAsQs8sQh", 1);
	compare_test("6c6s7dJc6h", "Jh5d2cJdJs", -1);

	// Straight
	compare_test("5h9d6c8s7s", "7c3s5s6s4h", 1);
	compare_test("Ah5d2c4d3s", "Tc6s8c7s9h", -1);
	compare_test("ThJd8c9d7s", "7c8sTdJh9h", 0);
	compare_test("3h2d5c4dAs", "4c5s3d2hAh", 0);

	// Flush
	compare_test("QdAd6dTd7d", "QhAh5h7hTh", 1);
	compare_test("QdAd6dTd7d", "QhAh5h8hTh", -1);
	compare_test("QdAd6dJd7d", "QhAh5h8hTh", 1);
	compare_test("QdAd6dJd7d", "KhAh5h8hTh", -1);
	compare_test("Qd2d6dJd7d", "Kh4h5h8hTh", -1);
	compare_test("QdTd5d8d4d", "Qh4h5h8hTh", 0);

	// Full house
	compare_test("AhAd5cAs5h", "Qc5sQs5dQh", 1);
	compare_test("2h5c2s2c5h", "Qc5sQs5dQh", -1);

	// Four of a kind
	compare_test("4sKs4c4d4h", "QdQsQc5dQh", -1);
	compare_test("Ts3cThTdTc", "5h5c2s5s5d", 1);

	// Straight flush
	compare_test("5h9h6h8h7h", "7c3c5c6c4c", 1);
	compare_test("Ad5d2d4d3d", "Ts6s8s7s9s", -1);
	compare_test("ThJh8h9h7h", "7s8sTsJs9s", 0);
	compare_test("3c2c5c4cAc", "4d5d3d2dAd", 0);

	// Royal flush
	compare_test("KhJhQhThAh", "JcQcTcAcKc", 0);
}

//void print_combinations()
//{
	//card_t deck[DECK_SIZE];
	//char card_buffer[3];

	// Initialize deck.
	// Ranks must be first, then suits for INDEX to work.
	//for (rank_t r = TWO; r <= ACE; r++)
	//{
	//	for (suit_t s = CLUBS; s <= SPADES; s++)
	//	{
	//		int i = INDEX(r, s);
	//		deck[i].rank = r;
	//		deck[i].suit = s;
	//	}
	//}

	//bool more;
	//combination_info_t* info = initialize(deck, DECK_SIZE, BOARD_SIZE, BUFFER_SIZE);

	//do
	//{
	//	more = combinations(info);

	//	for (int i = 0; i < info->combination_count; i++)
	//	{
	//		for (int j = 0; j < info->combination_size; j++)
	//		{
	//			card_to_string(&info->combination_buffer[i * info->combination_size + j], card_buffer);
				//printf("%s", card_buffer);
	//			fwrite(card_buffer, 1, 2, output);
	//		}

			//printf("\n");
	//		fwrite("\n", 1, 1, output);
	//	}
	//}
	//while (more);

	//dispose(info);
//}

void logger()
{
	board_t board;
	card_t hole_cards[2];
	char card_buffer[3];

	FILE* output = fopen("C:\\Users\\Gabriel\\Desktop\\log.txt", "w");

	string_to_cards("KsAc2s3c7d", board);
	string_to_cards("Ts2c", hole_cards);

	FSM_reset_rules(HOLDEM);
	FSM_reset_board_cards(board);
	FSM_reset_hole_cards(hole_cards, 2);

	while (FSM_next(board))
	{
		for (int i = 0; i < BOARD_SIZE; i++)
		{
			card_to_string(&board[i], card_buffer);
			fwrite(card_buffer, 1, 2, output);
		}

		fwrite("\n", 1, 1, output);
	}

	fclose(output);
}

void eval_tests()
{
	//eval_test(HOLDEM, 2, "KsAc2s3c7d", NULL, "3h5h", "Ts2c", 100.0d, 0.0d, 0.0d, 0.0d, 100.0d, 0.0d);
	eval_test(HOLDEM, 2, NULL, NULL, "3h5h", "Ts2c", 46.06d, 51.92d, 2.02d, 51.92d, 46.06d, 2.02d);
}

int main()
{
	//rank_tests();
	//comparation_tests();
	//combination_tests();
	eval_tests();
	//logger();
}
