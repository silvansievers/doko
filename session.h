#ifndef SESSION_H
#define SESSION_H

#include "cards.h"

#include "rng.h"

class GameType;
class GameState;
class Options;
class Player;

class Session {
private:
    Options &options;
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
