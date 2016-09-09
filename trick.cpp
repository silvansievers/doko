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

#include "trick.h"

#include "game_type.h"

#include <cassert>
#include <iostream>

using namespace std;

Trick::Trick(const GameType *game_type_, int first_player_)
    : game_type(game_type_), first_player(first_player_) {
}

int Trick::get_size() const {
    int result = 0;
    for (int i = 0; i < 4; ++i) {
        if (cards[i] != no_card)
            ++result;
    }
    return result;
}

void Trick::get_legal_cards_for_player(int player, vector<Move> &legal_moves, Cards players_cards) const {
    vector<Card> legal_cards;
    if (empty()) { // new trick => player can play any of his cards
        players_cards.get_single_cards(legal_cards);
    } else {
        assert(cards[first_player] != no_card);
        Cards trick_suit = game_type->get_suit(cards[first_player]);
        Cards players_cards_for_current_suit = players_cards.get_intersection(trick_suit);
        if (!players_cards_for_current_suit.empty()) // if player has one or more cards of the trick's suit, he has to play one of it
            players_cards_for_current_suit.get_single_cards(legal_cards);
        else // otherwise he can play any of his cards (this includes trump or non-trump)
            players_cards.get_single_cards(legal_cards);
    }
    for (size_t i = 0; i < legal_cards.size(); ++i) {
        legal_moves.push_back(Move(legal_cards[i]));
    }
}

void Trick::set_card(int player, Card card) {
    cards[player] = card;
}

int Trick::taken_by() const {
    //assert(get_size() == 4); // allow BeliefGameState to compute who wins the trick even if has not been completed (for heuristic move computation) or if some card slots are filled by dummy no_card cards
    assert(!empty());
    Card highest_card_so_far = cards[first_player]; // first card is the highest so far
    int player_it = first_player;
    int winning_player = first_player;
    Cards trump_suit = game_type->get_trump_suit();
    for (int i = 0; i < 3; ++i) { // iterate over the three other cards
        player_it = next_player(player_it);
        Card card = cards[player_it];
        if (card == no_card) // skip a dummy card
            continue;
        if (trump_suit.contains_card(highest_card_so_far)) { // current highest card is a trump card
            if (!trump_suit.contains_card(card)) // player didn't play trump, he can't win the trick
                continue;
            if (game_type->get_trump_rank(card) >= game_type->get_trump_rank(highest_card_so_far)) {
                // card's rank is lower or equal (higher number) than highest card's rank
                continue;
            }
            // else
            highest_card_so_far = card;
            winning_player = player_it;
            continue;
        }

        // else: no trump in the trick so far
        assert(game_type->get_suit(cards[first_player]).contains_card(highest_card_so_far)); // the highest card so far must be the highest card of the trick's suit (which is determined by first player's card)
        if (trump_suit.contains_card(card)) { // card is trump, i.e. it wins
            highest_card_so_far = card;
            winning_player = player_it;
            continue;
        }
        if (!game_type->get_suit(cards[first_player]).contains_card(card)) // player didn't play the same suit (but no trump)
            continue;
        // player played the same suit, compare ranks
        if (game_type->get_non_trump_rank(card) >= game_type->get_non_trump_rank(highest_card_so_far)) {
            // card's rank is lower or equal (higher number) than highest card's rank
            continue;
        }
        // else
        highest_card_so_far = card;
        winning_player = player_it;
        continue;
    }
    return winning_player;
}

int Trick::get_value_() const {
    assert(get_size() == 4);
    int value = 0;
    for (size_t i = 0; i < 4; ++i)
        value += cards[i].get_value();
    return value;
}

int Trick::get_special_points_for_trick_winner(bool dump, const int players_team[4], int trick_taken_by,
                                               int trick_value, bool last_trick = false) const {
    assert(trick_taken_by == taken_by());
    assert(trick_value == get_value_());
    int special_points_for_trick_winner = 0;
    for (int i = 0; i < 4; ++i) {
        if (cards[i] == DA || cards[i] == DA_) { // i played a fox
            if (players_team[i] != players_team[trick_taken_by]) { // i is not in the same team as the player who took the trick
                if (dump)
                    cout << "player " << trick_taken_by << " caught a fox!" << endl;
                ++special_points_for_trick_winner;
            }
        }
    }
    if (trick_value >= 40) {
        if (dump)
            cout << "player " << trick_taken_by << " made a doppelkopf!" << endl;
        ++special_points_for_trick_winner;
    }
    if (last_trick) {
        if (cards[trick_taken_by] == CJ || cards[trick_taken_by] == CJ_) {
            if (dump)
                cout << "player " << trick_taken_by << " won the last trick with a charlie!" << endl;
            ++special_points_for_trick_winner;
        }
    }
    if (dump && special_points_for_trick_winner != 0)
        cout << endl;
    return special_points_for_trick_winner;
}

void Trick::dump() const {
    cout << "trick's content:" << endl;
    int player = first_player;
    for (size_t i = 0; i < 4; ++i) {
        cout << "player " << player << "'s card: " << cards[player] << endl;
        player = next_player(player);
    }
}

Cards Trick::get_trick_suit() const {
    if (empty())
        return Cards();
    else
        return game_type->get_suit(cards[first_player]);
}
