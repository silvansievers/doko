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

#ifndef GAME_TYPE_H
#define GAME_TYPE_H

#include "cards.h"

#include <cassert>
#include <iostream>
#include <vector>

class GameType {
private:
    enum game_t {
        REGULAR,
        MARRIAGE,
        DIAMONDS_SOLO,
        HEARTS_SOLO,
        SPADES_SOLO,
        CLUBS_SOLO,
        JACKS_SOLO,
        QUEENS_SOLO,
        ACES_SOLO
    };
    game_t type;
    Cards trump_suit;
    std::vector<Cards> non_trump_suits;
    unsigned int card_to_trump_rank[48]; // maps a trump card to its rank (all other entries for non trump cards are invalid/not set!)
    Cards card_to_suit[48]; // maps a card to its suit (either trump_suit or one of non_trump_suits)
public:
    explicit GameType(int type);
    Cards get_trump_suit() const {
        return trump_suit;
    }
    const std::vector<Cards> &get_non_trump_suits() const {
        return non_trump_suits;
    }
    unsigned int get_trump_rank(Card card) const {
        return card_to_trump_rank[card.get_index()];
    }
    unsigned int get_non_trump_rank(Card card) const;
    Cards get_suit(Card card) const {
        return card_to_suit[card.get_index()];
    }

    bool operator==(const GameType &rhs) const {
        return type == rhs.type;
    }
    bool operator!=(const GameType &rhs) const {
        return type != rhs.type;
    }
    friend std::ostream &operator<<(std::ostream &out, const GameType &game_type);

    unsigned int get_rank(Card card) const { // added for BeliefGameState::get_best_move_index
        if (trump_suit.contains_card(card))
            return get_trump_rank(card);
        else
            return get_non_trump_rank(card);
    }
    bool is_trump(Card card) const { // added for BeliefGameState::get_best_move_index
        return trump_suit.contains_card(card);
    }
};

extern const GameType regular;
extern const GameType marriage;
extern const GameType diamonds_solo;
extern const GameType hearts_solo;
extern const GameType spades_solo;
extern const GameType clubs_solo;
extern const GameType jacks_solo;
extern const GameType queens_solo;
extern const GameType aces_solo;

#endif
