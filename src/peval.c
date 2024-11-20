#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "poker.h"
#include "eval.h"
#include "misc.h"

enum rules_t GetRules (int nHoleCards)
{
	switch (nHoleCards)
	{
		case 2: return HOLDEM;
		case 4: return OMAHA;
		case 5: return OMAHA5;
		case 6: return OMAHA6;
	}
}

void main (int argc, char* argv[])
{
	struct eval_t evalData;
	int opt;

	memset (&evalData, 0, sizeof (struct eval_t));

	while ((opt = getopt (argc, argv, "b:d:")) != -1)
	{
		switch (opt)
		{
			case 'b':
				if (StringToCards (optarg, evalData.boardCards) == NULL)
				{
					printf ("Invalid board cards: %s\n", optarg);
					return;
				}

				evalData.nBoardCards = strlen (optarg) / 2;

				break;

			case 'd':
				if (StringToCards (optarg, evalData.deadCards) == NULL)
				{
					printf ("Invalid dead cards: %s\n", optarg);
					return;
				}

				evalData.nDeadCards = strlen (optarg) / 2;

				break;

			default:
				printf ("peval [-b board cards] [-d dead cards] hole cards by player\n");
				printf ("\n");
				printf ("Examples:\n");
				printf ("\n");
				printf ("Hold'em: peval [-bKsTs2h4c7d] [-dAcTc] 9d7h 5c6s Jd9h\n");
				printf ("Omaha:   peval [-b6h6dTc] 8c3dTh4d KcKsAd2h\n");
				printf ("Omaha5:  peval Qh4hJsKc9c 4s5s8hThAc 3d4d5hTc7s\n");
				printf ("Omaha6:  peval [-b2s2c7h8d] [-d5sTd8s9d] AdQs4s8cQd5c JsJd6h4h9cAs\n");
				printf ("\n");
				return;
		}
	}

	if (optind >= argc) {
		printf ("Missing hole cards.\n");
		return;
	}

	int nPlayers = 0;

	for (; optind + nPlayers < argc; nPlayers++)
	{
		if (StringToCards (argv[optind + nPlayers], evalData.holeCards[nPlayers]) == NULL)
		{
			printf ("Invalid hole cards: %s\n", argv[optind + nPlayers]);
			return;
		}

		int nHoleCards = strlen (argv[optind + nPlayers]) / 2;

		if (nHoleCards != 2 && nHoleCards != 4 && nHoleCards != 5 && nHoleCards != 6)
		{
			printf ("Invalid number of hole cards: %d\n", nHoleCards);
			return;
		}

		if (evalData.nHoleCards > 0 && nHoleCards != evalData.nHoleCards)
		{
			printf ("Hole cards count mismatch: %d and %d\n", nHoleCards, evalData.nHoleCards);
			return;
		}

		if (evalData.nHoleCards == 0)
		{
			evalData.nHoleCards = nHoleCards;
		}
	}

	if (nPlayers < 2 || nPlayers > 10)
	{
		printf ("Invalid number of players: %d\n", nPlayers);
		return;
	}

	evalData.rules = GetRules (evalData.nHoleCards);
	evalData.nPlayers = nPlayers;

	Eval (&evalData);

	if (evalData.errors != 0)
	{
		if (evalData.errors & DUPLICATED_CARD_FLAG)
		{
			printf ("Duplicated card.\n");
		}

		if (evalData.errors & INSUFFICIENT_COMBINATION_CARDS)
		{
			printf ("Insufficient combination cards.\n");
		}

		if (evalData.errors & INVALID_BOARD_CARDS_COUNT)
		{
			printf ("Invalid board cards count.\n");
		}

		if (evalData.errors & INVALID_POKER_RULES)
		{
			printf ("Invalid poker rules.\n");
		}

		return;
	}

	printf ("Poker game: %s\n", RulesToString (evalData.rules));
	printf ("Evaluated boards: %d\n", evalData.nBoards);
	printf ("\n");

	for (int i = 0; i < nPlayers; i++)
	{
		int loses = evalData.nBoards - evalData.equities[i].wins - evalData.equities[i].ties;

		printf ("Data for player %d (%s):\n", i + 1, argv[optind + i]);
		printf (
			"Wins: %d (%2.2f). Loses: %d (%2.2f). Ties: %d (%2.2f)\n",
			evalData.equities[i].wins,
			evalData.equities[i].winProbability * 100.0d,
			loses,
			evalData.equities[i].loseProbability * 100.0d,
			evalData.equities[i].ties,
			evalData.equities[i].tieProbability * 100.0d
			);
		printf ("\n");
	}
}
