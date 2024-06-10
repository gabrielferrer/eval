#include <stddef.h>
#include <stdio.h>
#include "enum.h"
#include "eval.h"
#include "misc.h"

hand_rank_result_t r;

void hand_rank_test(char* combination_string, hand_rank_t hr)
{
	combination_t combination;
	hand_rank_result_t result;

	if (!string_to_combination(combination_string, combination))
	{
		printf("Invalid input: %s", combination_string);
		return;
	}

	hand_rank(combination, &result);

	printf("Combination: %s. Expected: %s. Actual: %s\n", combination_string, hand_rank_to_string(hr), hand_rank_to_string(result.hand_rank));
}

void compare_test(char* combination_string1, char* combination_string2, int e)
{
	combination_t combination1;
	combination_t combination2;
	hand_rank_result_t result1;
	hand_rank_result_t result2;

	if (!string_to_combination(combination_string1, combination1))
	{
		printf("Invalid input: %s", combination_string1);
		return;
	}

	if (!string_to_combination(combination_string2, combination2))
	{
		printf("Invalid input: %s", combination_string2);
		return;
	}

	printf("1st. combination: %s. 2nd. combination: %s. Expected: %d. Actual: %d\n", combination_string1, combination_string2, e, compare(combination1, combination2));
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

#define BUFFER_SIZE 10000

void combination_tests()
{
	card_t deck[DECK_SIZE];
	char card_buffer[3];

	// Initialize deck.
	// Ranks must be first, then suits for INDEX to work.
	for (rank_t r = TWO; r <= ACE; r++)
	{
		for (suit_t s = CLUBS; s <= SPADES; s++)
		{
			int i = INDEX(r, s);
			deck[i].rank = r;
			deck[i].suit = s;
		}
	}

	bool more;
	combination_info_t* info = initialize(deck, DECK_SIZE, COMBINATION_SIZE, BUFFER_SIZE);
	FILE* output = fopen("C:\\Users\\Gabriel\\Desktop\\combinations.txt", "w");

	do
	{
		more = combinations(info);

		for (int i = 0; i < info->combination_count; i++)
		{
			for (int j = 0; j < info->combination_size; j++)
			{
				card_to_string(&info->combination_buffer[i * info->combination_size + j], card_buffer);
				//printf("%s", card_buffer);
				fwrite(card_buffer, 1, 2, output);
			}

			//printf("\n");
			fwrite("\n", 1, 1, output);
		}
	}
	while (more);

	fclose(output);
	dispose(info);
}

int main()
{
	//rank_tests();
	//comparation_tests();
	combination_tests();
}
