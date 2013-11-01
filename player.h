#ifndef PLAYER_H
#define PLAYER_H

#include "cards.h"
#include "move.h"

#include <vector>

class Player {
protected:
    const int id; // player's unique number, from 0 to 3
    Cards cards;
public:
    explicit Player(int player_number);
    virtual ~Player() {}
    virtual void set_cards(Cards cards);

    virtual size_t ask_for_move(const std::vector<Move> &legal_moves) = 0;
    void invalid_move() const;
    virtual void inform_about_move(int player, const Move &move);
    virtual void inform_about_game_end(const int players_game_points[4]) {}
};

#endif
