#ifndef CARD_ASSIGNMENT_H
#define CARD_ASSIGNMENT_H

#include "cards.h"
#include "rng.h"

#include <vector>

/**
This algorithm is designed to randomly assign the remaining cards in the game to all other players than the one owning the Uct and CardAssignment instance. During the game, card suits that a player cannot have anymore are stored. This also includes queens of clubs (if a player announced kontra or two other players announced re). Furthermore, there is an array specifying if a player needs a queen of clubs (this is only of use if it is not known to the player which queen of clubs this player needs; in the latter case this specific queen of clubs would just be forbidden for all other players which then serves the same purpose).

The algorithm works as follows: given all the above information, each remaining card (which is to be distributed) gets associated with a list of players that can actually have it. The algorithm consists of one big loop over all remaining cards which always assigns exactly one card to a player and afterwards checks if there is a player who now has all cards he needs, in which case he gets removed from all lists (mapping a card to a set of players that can have it). If this is not the case, then there is another check being executed: If a player can only have exactly as many cards as he needs, he must get all of these cards. This is realised by removing all other players that can have these cards. Afterwards the loop is repeated.
Now the loop works as follows: for each iteration, "size_to_check" is fixed for 1, 2 or 3, meaning that when size to check equals 1, then only cards which may only be assigned to one player are considered (and thus assigned to this one player). For the first iteration of the algorithm, size_to_check is set to 1. If there are no more cards being assigned this way, size_to_check is being incremeted by 1. Only when this happens for the first time and there are still one or more players who need to get a queen of clubs, then the queen(s) of clubs are (randomly) assigned to this/those players. Again, as after every card assignment, the two checks mentioned above are executed.
In iterations where size_to_check is set to 2 or 3, the card being currently considered is randomly assigned to one of those 2 or 3 players, then running the checks again. Whenever one of those checks succeeds (i.e. a player is completely removed from all lists because he got all cards he needs or a player needs to get all cards he may get), size_to_check is decremented by 1 and the loop is repeated.

This is not a strictly uniform random assignment but should get quite close to it.
*/

class BeliefGameState;
class Options;

class CardAssignment {
private:
    const Options &options;
    mutable RandomNumberGenerator rng;
    std::vector<Card> remaining_cards;
    std::vector<std::vector<int> > card_to_players_who_can_have_it;
public:
    CardAssignment(const Options &options, const BeliefGameState &state, Cards players_cards);
    void assign_cards_to_players(BeliefGameState &state) const;
};

#endif
