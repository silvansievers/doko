/*
  doko is a C++ doppelkopf program with an integrated UCT player.
  Copyright (c) 2011-2016 Silvan Sievers
  For questions, please write to: silvan.sievers@unibas.ch

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "card_assignment.h"

#include "belief_game_state.h"
#include "options.h"

#include <cassert>
#include <iostream>

using namespace std;

static void remove_player_who_got_all_cards(int player, vector<vector<int> > &card_to_players_who_can_have_it);
static void remove_players_from_card_which_is_uniquely_assigned(int player, int index_of_card_that_player_must_have, vector<vector<int> > &card_to_players_who_can_have_it);
static bool check_if_player_needs_as_many_cards_as_he_can_get(int player, const vector<Card> &remaining_cards, vector<vector<int> > &card_to_players_who_can_have_it, int cards_count);

CardAssignment::CardAssignment(const Options &options_, const BeliefGameState &state, Cards players_cards) : options(options_), rng(2011) {
    assert(players_cards.size() >= 1);
    Cards played_cards = state.get_played_cards();
    int player_to_move = state.get_player_to_move();
    const int *players_cards_count = state.get_players_cards_count();

    // compute the set of remaining cards
    remaining_cards.reserve(48U - played_cards.size() - players_cards.size());
    for (int i = 0; i < 48; ++i) {
        Card card(i);
        // only insert cards which have not been played yet and which are not owned by player
        if (!played_cards.contains_card(card) && !players_cards.contains_card(card))
            remaining_cards.push_back(Card(i));
    }
    assert(remaining_cards.size() == 48U - played_cards.size() - players_cards.size());
    assert(remaining_cards.size() >= 3); // the last time Uct should be computed is when the last player has 2 cards and all other have one, i.e. for this player there are 3 other cards remaining in the game

    // for each player compute cards which he cannot have
    const Cards *cards_that_players_cannot_have = state.get_cards_that_players_cannot_have();
    Cards remaining_cards_that_players_cannot_have[4];
    for (int i = 0; i < 4; ++i) {
        if (i == player_to_move)
            continue;
        remaining_cards_that_players_cannot_have[i] = cards_that_players_cannot_have[i];
        // remove played_cards from remaining_cards_that_players_cannot_have because they are not included in remaining_cards
        remaining_cards_that_players_cannot_have[i].remove_cards(played_cards);
        // remove player's cards from remaining_cards_that_players_cannot_have because they are not included in remaining_cards
        remaining_cards_that_players_cannot_have[i].remove_cards(players_cards);
        assert(static_cast<unsigned int>(remaining_cards_that_players_cannot_have[i].size()) < remaining_cards.size()); // player needs to be able to get at least one card
        // now remaining_cards_that_players_cannot_have may only contain cards which are also contained in remaining_cards
        vector<Card> single_cards;
        remaining_cards_that_players_cannot_have[i].get_single_cards(single_cards);
        for (size_t j = 0; j < single_cards.size(); ++j) {
            bool contains_card = false;
            for (size_t k = 0; k < remaining_cards.size(); ++k)
                if (single_cards[j] == remaining_cards[k])
                    contains_card = true;
                assert(contains_card);
        }
    }

    // compute a mapping that maps each card (index as in remaining_cards) to a list of players who can have it
    card_to_players_who_can_have_it.resize(remaining_cards.size());
    for (size_t i = 0; i < remaining_cards.size(); ++i) {
        Card card = remaining_cards[i];
        for (int j = 0; j < 4; ++j) {
            if (j == player_to_move)
                continue;
            if (!remaining_cards_that_players_cannot_have[j].contains_card(card))
                card_to_players_who_can_have_it[i].push_back(j);
        }
    }

    // remove players who do not need to get any cards and/or remove all players except the one who must get a specific card because he cannot get all of the other cards or because he needs to get all cards he may get
    for (int i = 0; i < 4; ++i) {
        if (i == player_to_move)
            continue;
        if (players_cards_count[i] == 0)
            remove_player_who_got_all_cards(i, card_to_players_who_can_have_it);
        check_if_player_needs_as_many_cards_as_he_can_get(i, remaining_cards, card_to_players_who_can_have_it, players_cards_count[i]);
    }
}

// remove a player entirely from the list of card_to_players_who_can_have_it
void remove_player_who_got_all_cards(int player, vector<vector<int> > &card_to_players_who_can_have_it) {
    for (size_t i = 0; i < card_to_players_who_can_have_it.size(); ++i) {
        vector<int> &players = card_to_players_who_can_have_it[i];
        for (vector<int>::iterator it = players.begin(); it != players.end(); ++it) {
            if (*it == player) {
                players.erase(it);
                break;
            }
        }
    }
}

// remove all players from card_to_players_who_can_have_it for a given card (because that player should get this specific card)
void remove_players_from_card_which_is_uniquely_assigned(int player, int index_of_card_that_player_must_have, vector<vector<int> > &card_to_players_who_can_have_it) {
    vector<int> &players = card_to_players_who_can_have_it[index_of_card_that_player_must_have];
    vector<int>::iterator it = players.begin();
    while (it != players.end()) {
        if (*it != player) {
            it = players.erase(it);
        } else {
            ++it;
        }
    }
}

// if the number of cards a player can get equals the number of cards he still needs to get, then remove all other players from card_to_players_who_can_have_it such that when assigning these cards, only the player who has to get them is being considered.
bool check_if_player_needs_as_many_cards_as_he_can_get(int player, const vector<Card> &remaining_cards, vector<vector<int> > &card_to_players_who_can_have_it, int number_of_cards_player_still_needs) {
    int number_of_cards_player_can_have = 0;
    vector<int> indices_of_cards_that_player_must_have;
    for (size_t i = 0; i < remaining_cards.size(); ++i) {
        if (remaining_cards[i] == no_card)
            continue;
        const vector<int> &players_for_card = card_to_players_who_can_have_it[i];
        bool player_can_have_card = false;
        for (size_t j = 0; j < players_for_card.size(); ++j) {
            if (players_for_card[j] == player) {
                player_can_have_card = true;
                break;
            }
        }
        if (player_can_have_card) {
            ++number_of_cards_player_can_have;
            indices_of_cards_that_player_must_have.push_back(i);
        }
    }
    assert(number_of_cards_player_can_have > 0);
    if (number_of_cards_player_can_have == number_of_cards_player_still_needs) {
        for (size_t i = 0; i < indices_of_cards_that_player_must_have.size(); ++i)
            remove_players_from_card_which_is_uniquely_assigned(player, indices_of_cards_that_player_must_have[i], card_to_players_who_can_have_it);
        return true;
    }
    return false;
}

void CardAssignment::assign_cards_to_players(BeliefGameState &state) const {
    // need to make a copy because it will be changed later
    vector<Card> _remaining_cards(remaining_cards);
    vector<vector<int> > _card_to_players_who_can_have_it(card_to_players_who_can_have_it);
    const int *players_cards_count = state.get_players_cards_count();
    const bool* temp = state.get_players_must_have_queen_of_clubs();
    bool players_must_have_queen_of_clubs[4];
    int num_players_that_need_queen_of_clubs = 0;
    for (int i = 0; i < 4; ++i) {
        players_must_have_queen_of_clubs[i] = temp[i];
        if (temp[i])
            ++num_players_that_need_queen_of_clubs;
    }
    Cards players_assigned_cards[4];

    // check for cards that can only be assigned to 1 players. repeat as long as there are, then increment size_to_check to 2. assign to a random player and if this player then has all his cards, restart all over with size_to_check = 1. this continues until size_to_check = 3 and repeat = false.
    size_t size_to_check = 1;
    while (size_to_check <= 3) {
        bool repeat = false;
        for (size_t i = 0; i < _remaining_cards.size(); ++i) {
            if (_remaining_cards[i] == no_card) // card has been assigned already
                continue;
            const vector<int> &players_for_card = _card_to_players_who_can_have_it[i];
            if (players_for_card.size() != size_to_check) // only test for the current size
                continue;
            assert(players_for_card.size() > 0);
            int player_to_assign_card_to = -1;
            assert(players_assigned_cards[players_for_card[0]].size() < players_cards_count[players_for_card[0]]);
            bool assigning_queen_of_clubs = false;
            if (size_to_check == 1) {
                player_to_assign_card_to = players_for_card[0];
                if (_remaining_cards[i] == CQ || _remaining_cards[i] == CQ_) {
                    /* in the case that a queen of clubs may only be assigned to exactly one player, this should not additionally be stored in players_must_have_queen_of_clubs. Exception: in the case of a silent solo player having announced re, he "must have a queen of clubs", if then additionally two other players say kontra and the last player is the player who is doing this card assignment, this player knows for sure that the one player needs both queens of clubs. thus only this player may have both queens of clubs assigned to AND players_must_have_queen_of_clubs is true for this player nevertheless, because it is not yet a global information for all players that this player must have both queens of clubs (in which case players_must_have_queen_of_clubs would be set to false). therefore the assertion is weakened to allow for the actual player who is getting assigned a queen of clubs to must have a queen of clubs. */
                    for (int j = 0; j < 4; ++j) {
                        if (j == player_to_assign_card_to && players_must_have_queen_of_clubs[player_to_assign_card_to]) {
                            players_must_have_queen_of_clubs[player_to_assign_card_to] = false;
                            --num_players_that_need_queen_of_clubs;
                        } else if (j != player_to_assign_card_to)
                            assert(!players_must_have_queen_of_clubs[j]);
                    }
                }
            }
            else {
                // when being here for the first time, all players who only could have one card got it. now need to assign a queen of clubs to the player(s) who need(s) one, because otherwise, the player(s) may already have as many cards assigned as he/they need/s and thus cannot have the queen of clubs anymore.
                if (num_players_that_need_queen_of_clubs > 0) {
                    --num_players_that_need_queen_of_clubs;
                    assigning_queen_of_clubs = true;
                    for (int j = 0; j < 4; ++j) {
                        if (j == state.get_player_to_move())
                            continue;
                        if (players_must_have_queen_of_clubs[j]) {
                            players_must_have_queen_of_clubs[j] = false;
                            player_to_assign_card_to = j;
                            assert(players_assigned_cards[player_to_assign_card_to].size() < players_cards_count[player_to_assign_card_to]);
                            // find index i of _remaining_cards for the first queen of clubs and assign it to the player who needs one (given we are in the else block with size_to_check at least 2, there are at least two candidate players for each queen of clubs and thus one can just arbitrarily assign the first one to the (first) player who needs it)
                            while (_remaining_cards[i] != CQ && _remaining_cards[i] != CQ_) {
                                ++i;
                            }
                            break;
                        }
                    }
                    assert(player_to_assign_card_to != -1);
                } else {
                    assert(players_assigned_cards[players_for_card[1]].size() < players_cards_count[players_for_card[1]]);
                    if (size_to_check == 3)
                        assert(players_assigned_cards[players_for_card[2]].size() < players_cards_count[players_for_card[2]]);
                    int random = rng.next(size_to_check);
                    player_to_assign_card_to = players_for_card[random];
                }
            }
            assert(player_to_assign_card_to != state.get_player_to_move()); // player to move has his own cards, he must never be considered in this method!
            players_assigned_cards[player_to_assign_card_to].add_card(_remaining_cards[i]); // assign card
            _remaining_cards[i] = no_card; // set card as assigned
            if (players_assigned_cards[player_to_assign_card_to].size() == players_cards_count[player_to_assign_card_to]) {
                // player got all his cards now, need to restart the for loop (the test for players_for_card.size() == 1 might trigger more often now)
                remove_player_who_got_all_cards(player_to_assign_card_to, _card_to_players_who_can_have_it);
                repeat = true;
                if (size_to_check == 2 || size_to_check == 3) // one player got all his cards now -> re-check next round with size_to_check decremented by 1
                    --size_to_check;
                break;
            }
            if (assigning_queen_of_clubs) // when assigning a queen of clubs, i was on 0 for the iteration and thus should be on 0 again such that the remaining card at index 0 also gets assigned to a player
                i = -1;
            // check if a player needs to get a specific cards because he cannot get all others or all cards he needs because there is no choice
            bool changed_something = false;
            for (int j = 0; j < 4; ++j) {
                if (j == state.get_player_to_move())
                    continue;
                if (players_assigned_cards[j].size() < players_cards_count[j]) { // if players have all cards assigned then they are removed from card_to_players_who_can_have_it, thus the check would fail
                    if (check_if_player_needs_as_many_cards_as_he_can_get(j, _remaining_cards, _card_to_players_who_can_have_it, players_cards_count[j] - players_assigned_cards[j].size()))
                        changed_something = true;
                }
            }
            if (changed_something) {
                repeat = true;
                if (size_to_check == 2 || size_to_check == 3) // possibly some players have been removed from card_to_players_who_can_have_it -> re-check next round with size_to_check decremented by 1 (this is enough because at most one player at a time is assigned a card. NOTE: if one player can have cards 1, 2 and 3 (needs 2 cards), another player can have 3, 4 and 5 (needs 2 cards), and a third player gets assigned card 3, then the two other players must get assigned their cards. but then anyway the remaining player must get all remaining cards, which is why decrementing size_to_check only by 1 and not resetting it to 1 neccessarily is enough.)
                    --size_to_check;
                break;
            }
        }
        if (!repeat)
            ++size_to_check;
    }

    //cout << "assigned cards to players in the following way:" << endl;
    for (int i = 0; i < 4; ++i) {
        //cout << players_assigned_cards[i] << endl;
        if (i == state.get_player_to_move())
            assert(players_assigned_cards[i].empty());
        else
            assert(players_assigned_cards[i].size() == players_cards_count[i]);
    }

    state.set_other_players_cards(players_assigned_cards);
}
