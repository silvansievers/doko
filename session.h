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

#ifndef SESSION_H
#define SESSION_H

#include "cards.h"

#include "rng.h"

class GameType;
class GameState;
class Options;
class Player;
class Timer;

class Session {
private:
    const Options &options;
    Timer *timer;
    Player *players[4];
    int players_points[4]; // accumulated points over all games
    bool played_compulsory_solo[4];
    RandomNumberGenerator rng;
    int first_player; // player to move first in the next game; when creating, this is set to be player 0 and will eventually change when playing more than 1 game.
    Cards cards[4]; // players' starting hands for the current game
    bool vorfuehrung; // true if a player is forced to play his compulsory solo (due to the number of remaining games being small)

    void play();
    void shuffle_cards();
    void set_cards() const; // distribute cards to players
    void check_vorfuehrung(int number_of_remaining_games); // test if the remaining games need all to be compulsory solos (i.e. vorfuehrung)
    void statistics() const; // print accumulated points
public:
    explicit Session(Options &options);
    ~Session();
};

#endif
