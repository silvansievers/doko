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

#ifndef ACTUAL_GAME_STATE_H
#define ACTUAL_GAME_STATE_H

#include "game_state.h"

/**
ActualGameState is only being used by Session. This is the class which represents the "real" game, i.e. which has all informations about all players and which also "plays" the game, i.e. asks players for moves, updates the played tricks, calculate the score points in the end and so on. The following are some remarks about the implementation of the game of Doppelkop:

Game type determination:
The first "Move" of the game will always be the question "do you want to play an immediate solo", meaning that the player announces his solo without waiting for the whole reservation procedure to go through. If there is at least one player who did not play his compulsory solo, then the first positioned player of these will be asked. If there is no such player, then the first positioned player is being asked. Afterwards, whenever a player says "gesund", it will be checked if with this information, any of the other remaining players now knows that he may immediately announce a solo because he has the highest priority and thus needs to be asked next (for example, player 0 and 2 did not play their compulsory solo yet, player 0 is starting the next game and neither announces an immediate solo nor has a reservation, i.e. he says "gesund". then player 2 knows that he can shorten the reservation procedure by immediately announcing his solo). If this player denies an immediate announcement, then the procedure needs to go back to the next player who needs to be asked for a regular reservation, thus sometimes making the reservation procedure and the order in which players are asked a bit confusing.

Announcements:
In order to avoid leaking information to players which they cannot have, it is important to always ask *all* players for an announcement as long as *any* player is still allowed to do an announcement. this may result in a move for a player where his only option is to say "no announcement", because he is not allowed anymore to make an announcement.
*/

class Player;

class ActualGameState : public GameState {
private:
    Player *players[4];
    bool played_compulsory_solo[4];

    void print_trick() const;
    size_t make_move(int player, const std::vector<Move> &move_options) const;
    bool shorten_reservation_procedure(int player);
    void ask_player_for_solo(int player); // ask player to decide the game type for the current game
    void determine_game_type(int first_player, bool vorfuehrung); // ask players for reservations and choses the game type
    void play(int first_player, bool vorfuehrung);
public:
    ActualGameState(const Options &options, Player *players[4], int first_player,
                    Cards cards[4], const bool played_compulsory_solo[4], bool vorfuehrung);
};

#endif
