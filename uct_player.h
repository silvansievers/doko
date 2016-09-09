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

#ifndef UCT_PLAYER_H
#define UCT_PLAYER_H

#include "player.h"

class BeliefGameState;
class Options;

class UctPlayer : public Player {
    const Options &options;
    BeliefGameState *current_belief_state;
    bool played_compulsory_solo[4];
    int first_player;
    bool vorfuehrung;
    int number_of_current_game;
    int counter;

    void check_vorfuehrung(int number_of_remaining_games);
public:
    UctPlayer(int player_number, const Options &options);
    ~UctPlayer();
    void set_cards(Cards cards);
    size_t ask_for_move(const std::vector<Move> &legal_moves);
    void inform_about_move(int player, const Move &move);
    void inform_about_game_end(const int players_game_points[4]);
};

#endif
