#ifndef RANDOM_PLAYER_H
#define RANDOM_PLAYER_H

#include "player.h"
#include "rng.h"

class RandomPlayer : public Player {
private:
    mutable RandomNumberGenerator rng;
public:
    explicit RandomPlayer(int player_number);
    size_t ask_for_move(const std::vector<Move> &legal_moves);
};

#endif
