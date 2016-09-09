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

#include "player.h"

#include <cassert>
#include <iostream>

using namespace std;

Player::Player(int player_number) : id(player_number) {
}

void Player::set_cards(Cards cards_) {
    cards = cards_;
}

void Player::invalid_move() const {
    cout << "player " << id << " somehow managed to return an invalid index" << endl;
    assert(false);
}

void Player::inform_about_move(int player, const Move &move) {
    if (move.is_card_move() && id == player) {
        cards.remove_card(move.get_card());
    }
}
