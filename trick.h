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

#ifndef TRICK_H
#define TRICK_H

#include "cards.h"
#include "move.h"

#include <vector>

class GameType;

class Trick {
private:
    const GameType *game_type;
    int first_player;
    Card cards[4]; // cards in the trick
public:
    Trick(const GameType *game_type, int first_player);
    void get_legal_cards_for_player(int player, std::vector<Move> &legal_moves, Cards players_cards) const;
    void set_card(int player, Card card);
    // Updated for BeliefGameState to allow to be called even when the trick is not completed. The player returned is the player who would win the trick so far
    int taken_by() const;
    int get_value_() const;
    // passing the trick's winner and the trick's value as arguments has the only goal of avoiding repeated calculations
    int get_special_points_for_trick_winner(bool dump, const int re_team[4], int trick_taken_by,
                                            int trick_value, bool last_trick) const;
    bool empty() const {
        return get_size() == 0;
    }
    bool completed() const {
        return get_size() == 4;
    }
    void dump() const;
    Cards get_trick_suit() const; // introduced for BeliefGameState
    bool operator==(const Trick &rhs) const { // added for BeliefGameState==
        assert(first_player == rhs.first_player);
        for (int i = 0; i < 4; ++i)
            assert(cards[i] == rhs.cards[i]);
        return true;
    }
    int get_size() const; // made public for BeliefGameState
};

#endif
