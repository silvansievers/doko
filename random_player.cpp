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

#include "random_player.h"

#include <cassert>
#include <iostream>

using namespace std;

RandomPlayer::RandomPlayer(int player_number) : Player(player_number), rng(2011) {
}

size_t RandomPlayer::ask_for_move(const vector<Move> &legal_moves) {
    if (legal_moves[0].is_question_move()) {
        assert(legal_moves[0].get_answer() == false);
        return 0;
    } else if (legal_moves[0].is_game_type_move()) { // should only be true in the case of vorfuehrung
        return rng.next(legal_moves.size());
    } else if (legal_moves[0].is_announcement_move()) {
        assert(legal_moves[0].get_announcement() == NONE);
        return 0;
    } else if (legal_moves[0].is_card_move()) {
        return rng.next(legal_moves.size());
    }
    assert(false);
    return 0;
}
