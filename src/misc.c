#include <stdlib.h>
#include <string.h>
#include "misc.h"

#ifdef WIN
__declspec(dllexport)
#endif
enum rank_t ToRank (char r)
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

#ifdef WIN
__declspec(dllexport)
#endif
enum suit_t ToSuit (char s)
{
	if (s == 'c') return CLUBS;
	if (s == 'd') return DIAMONDS;
	if (s == 'h') return HEARTS;
	if (s == 's') return SPADES;

	return NO_SUIT;
}

#ifdef WIN
__declspec(dllexport)
#endif
char ToRankString (enum rank_t r)
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

#ifdef WIN
__declspec(dllexport)
#endif
char ToSuitString (enum suit_t s)
{
	if (s == CLUBS) return 'c';
	if (s == DIAMONDS) return 'd';
	if (s == HEARTS) return 'h';
	if (s == SPADES) return 's';

	return '\0';	
}

#ifdef WIN
__declspec(dllexport)
#endif
char* HandRankToString (enum hand_rank_t handRank)
{
	if (handRank == HIGH_CARD) return "High Card";
	if (handRank == PAIR) return "Pair";
	if (handRank == TWO_PAIR) return "Two Pair";
	if (handRank == THREE_OF_A_KIND) return "Three of a Kind";
	if (handRank == STRAIGHT) return "Straight";
	if (handRank == FLUSH) return "Flush";
	if (handRank == FULL_HOUSE) return "Full House";
	if (handRank == FOUR_OF_A_KIND) return "For of a Kind";
	if (handRank == STRAIGHT_FLUSH) return "Straight Flush";
	if (handRank == ROYAL_FLUSH) return "Royal Flush";

	return NULL;
}

#ifdef WIN
__declspec(dllexport)
#endif
char* RulesToString (enum rules_t rules)
{
	if (rules == HOLDEM) return "Hold'em";
	if (rules == OMAHA) return "Omaha";
	if (rules == OMAHA5) return "Omaha-5";
	if (rules == OMAHA6) return "Omaha-6";

	return NULL;
}

#ifdef WIN
__declspec(dllexport)
#endif
struct card_t* StringToCards (char* cardsString, struct card_t* cards)
{
	if (cardsString == NULL)
	{
		return NULL;
	}

	int length = strlen (cardsString);

	if (length % 2 != 0)
	{
		return NULL;
	}

	enum rank_t r;
	enum suit_t s;

	for (int i = 0, j = 0; i < length; j++)
	{
		r = ToRank (cardsString[i++]);

		if (r == NO_RANK)
		{
			return NULL;
		}

		s = ToSuit (cardsString[i++]);

		if (s == NO_SUIT)
		{
			return NULL;
		}

		cards[j].rank = r;
		cards[j].suit = s;
	}

	return cards;
}

#ifdef WIN
__declspec(dllexport)
#endif
char* CardToString (struct card_t* card, char* destination)
{
	destination[0] = ToRankString (card->rank);
	destination[1] = ToSuitString (card->suit);
	destination[2] = '\0';

	return destination;
}

