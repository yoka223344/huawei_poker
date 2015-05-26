#include "poker.h"

#define SPAD_OFFSET			SPADES*13
#define CLUB_OFFSET			CLUBS*13
#define DIAM_OFFSET			DIAMONDS*13
#define HART_OFFSET			HEARTS*13

#define CARD_WIDTH			4

#define TOP_CARD_MASK		0x000F0000
#define SECOND_CARD_MASK	0x0000F000
#define FIFTH_CARD_MASK		0x0000000F

#define HANDTYPE_SHIFT		24

#define TOP_CARD_SHIFT		16
#define SECOND_CARD_SHIFT	12
#define THIRD_CARD_SHIFT	8
#define FOURTH_CARD_SHIFT	4
#define FIFTH_CARD_SHIFT	0

#define HANDTYPE_VALUE_STRAIGHTFLUSH	(STRAIGHT_FLUSH	<< HANDTYPE_SHIFT)
#define HANDTYPE_VALUE_STRAIGHT			(STRAIGHT		<< HANDTYPE_SHIFT)
#define HANDTYPE_VALUE_FLUSH			(FLUSH			<< HANDTYPE_SHIFT)
#define HANDTYPE_VALUE_FULLHOUSE		(FULL_HOUSE		<< HANDTYPE_SHIFT)
#define HANDTYPE_VALUE_FOUR_OF_A_KIND	(FOUR_OF_A_KIND	<< HANDTYPE_SHIFT)
#define HANDTYPE_VALUE_TRIPS			(TRIPS			<< HANDTYPE_SHIFT)
#define HANDTYPE_VALUE_TWOPAIR			(TWO_PAIR		<< HANDTYPE_SHIFT)
#define HANDTYPE_VALUE_PAIR				(PAIR			<< HANDTYPE_SHIFT)
#define HANDTYPE_VALUE_HIGHCARD			(HIGH_CARD		<< HANDTYPE_SHIFT)

extern unsigned short nBitsTable		[];
extern unsigned short straightTable		[];
extern unsigned short topCardTable		[];
extern unsigned int topFiveCardsTable	[];

static unsigned int card_eval(__int64 cards, int numberOfCards)
{
    unsigned int retval = 0, four_mask, three_mask, two_mask;

#if DEBUG
    // This functions supports 1-7 cards
    if (numberOfCards < 1 || numberOfCards > 7)
        return -1;
#endif

    // Seperate out by suit
    unsigned int sc = (unsigned int)((cards >> (CLUB_OFFSET)) & 0x1fff);
    unsigned int sd = (unsigned int)((cards >> (DIAM_OFFSET)) & 0x1fff);
    unsigned int sh = (unsigned int)((cards >> (HART_OFFSET)) & 0x1fff);
    unsigned int ss = (unsigned int)((cards >> (SPAD_OFFSET)) & 0x1fff);

    unsigned int ranks = sc | sd | sh | ss;
    unsigned int n_ranks = nBitsTable[ranks];
    unsigned int n_dups = ((unsigned int)(numberOfCards - n_ranks));

    /* Check for straight, flush, or straight flush, and return if we can
        determine immediately that this is the best possible hand 
    */
    if (n_ranks >= 5)
    {
        if (nBitsTable[ss] >= 5)
        {
            if (straightTable[ss] != 0)
                return HANDTYPE_VALUE_STRAIGHTFLUSH + (unsigned int)(straightTable[ss] << TOP_CARD_SHIFT);
            else
                retval = HANDTYPE_VALUE_FLUSH + topFiveCardsTable[ss];
        }
        else if (nBitsTable[sc] >= 5)
        {
            if (straightTable[sc] != 0)
                return HANDTYPE_VALUE_STRAIGHTFLUSH + (unsigned int)(straightTable[sc] << TOP_CARD_SHIFT);
            else
                retval = HANDTYPE_VALUE_FLUSH + topFiveCardsTable[sc];
        }
        else if (nBitsTable[sd] >= 5)
        {
            if (straightTable[sd] != 0)
                return HANDTYPE_VALUE_STRAIGHTFLUSH + (unsigned int)(straightTable[sd] << TOP_CARD_SHIFT);
            else
                retval = HANDTYPE_VALUE_FLUSH + topFiveCardsTable[sd];
        }
        else if (nBitsTable[sh] >= 5)
        {
            if (straightTable[sh] != 0)
                return HANDTYPE_VALUE_STRAIGHTFLUSH + (unsigned int)(straightTable[sh] << TOP_CARD_SHIFT);
            else
                retval = HANDTYPE_VALUE_FLUSH + topFiveCardsTable[sh];
        }
        else
        {
            unsigned int st = straightTable[ranks];
            if (st != 0)
                retval = HANDTYPE_VALUE_STRAIGHT + (st << TOP_CARD_SHIFT);
        };

        /* 
            Another win -- if there can't be a FH/Quads (n_dups < 3), 
            which is true most of the time when there is a made hand, then if we've
            found a five card hand, just return.  This skips the whole process of
            computing two_mask/three_mask/etc.
        */
        if (retval != 0 && n_dups < 3)
            return retval;
    }

    /*
        * By the time we're here, either: 
        1) there's no five-card hand possible (flush or straight), or
        2) there's a flush or straight, but we know that there are enough
            duplicates to make a full house / quads possible.  
        */
    switch (n_dups)
    {
        case 0:
            /* It's a no-pair hand */
            return HANDTYPE_VALUE_HIGHCARD + topFiveCardsTable[ranks];
        case 1:
            {
                /* It's a one-pair hand */
                unsigned int t, kickers;

                two_mask = ranks ^ (sc ^ sd ^ sh ^ ss);

                retval = (unsigned int) (HANDTYPE_VALUE_PAIR + (topCardTable[two_mask] << TOP_CARD_SHIFT));
                t = ranks ^ two_mask;      /* Only one bit set in two_mask */
                /* Get the top five cards in what is left, drop all but the top three 
                    * cards, and shift them by one to get the three desired kickers */
                kickers = (topFiveCardsTable[t] >> CARD_WIDTH) & ~FIFTH_CARD_MASK;
                retval += kickers;
                return retval;
            }

        case 2:
            /* Either two pair or trips */
            two_mask = ranks ^ (sc ^ sd ^ sh ^ ss);
            if (two_mask != 0)
            {
                unsigned int t = ranks ^ two_mask; /* Exactly two bits set in two_mask */
                retval = (unsigned int) (HANDTYPE_VALUE_TWOPAIR
                    + (topFiveCardsTable[two_mask]
                    & (TOP_CARD_MASK | SECOND_CARD_MASK))
                    + (topCardTable[t] << THIRD_CARD_SHIFT));

                return retval;
            }
            else
            {
                unsigned int t, second;
                three_mask = ((sc & sd) | (sh & ss)) & ((sc & sh) | (sd & ss));
                retval = (unsigned int) (HANDTYPE_VALUE_TRIPS + (topCardTable[three_mask] << TOP_CARD_SHIFT));
                t = ranks ^ three_mask; /* Only one bit set in three_mask */
                second = topCardTable[t];
                retval += (second << SECOND_CARD_SHIFT);
                t ^= (1U << (int)second);
                retval += (unsigned int) (topCardTable[t] << THIRD_CARD_SHIFT);
                return retval;
            }

        default:
            /* Possible quads, fullhouse, straight or flush, or two pair */
            four_mask = sh & sd & sc & ss;
            if (four_mask != 0)
            {
                unsigned int tc = topCardTable[four_mask];
                retval = (unsigned int) (HANDTYPE_VALUE_FOUR_OF_A_KIND
                    + (tc << TOP_CARD_SHIFT)
                    + ((topCardTable[ranks ^ (1U << (int)tc)]) << SECOND_CARD_SHIFT));
                return retval;
            };

            /* Technically, three_mask as defined below is really the set of
                bits which are set in three or four of the suits, but since
                we've already eliminated quads, this is OK */
            /* Similarly, two_mask is really two_or_four_mask, but since we've
                already eliminated quads, we can use this shortcut */

            two_mask = ranks ^ (sc ^ sd ^ sh ^ ss);
            if (nBitsTable[two_mask] != n_dups)
            {
                /* Must be some trips then, which really means there is a 
                    full house since n_dups >= 3 */
                unsigned int tc, t;
                three_mask = ((sc & sd) | (sh & ss)) & ((sc & sh) | (sd & ss));
                retval = HANDTYPE_VALUE_FULLHOUSE;
                tc = topCardTable[three_mask];
                retval += (tc << TOP_CARD_SHIFT);
                t = (two_mask | three_mask) ^ (1U << (int)tc);
                retval += (unsigned int) (topCardTable[t] << SECOND_CARD_SHIFT);
                return retval;
            };

            if (retval != 0) /* flush and straight */
                return retval;
            else
            {
                /* Must be two pair */
                unsigned int top, second;

                retval = HANDTYPE_VALUE_TWOPAIR;
                top = topCardTable[two_mask];
                retval += (top << TOP_CARD_SHIFT);
                second = topCardTable[two_mask ^ (1 << (int)top)];
                retval += (second << SECOND_CARD_SHIFT);
                retval += (unsigned int) ((topCardTable[ranks ^ (1U << (int)top) ^ (1 << (int)second)]) << THIRD_CARD_SHIFT);
                return retval;
            }
    }
}
