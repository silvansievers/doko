#ifndef HUMAN_PLAYER_H
#define HUMAN_PLAYER_H

#include "player.h"

class GameType;

class HumanPlayer : public Player {
    const GameType *game_type;
    size_t make_move(const std::vector<Move> &legal_moves) const;
    size_t make_card_move(const std::vector<Move> &legal_moves) const;
public:
    explicit HumanPlayer(int player_number);
    void set_cards(Cards cards);
    size_t ask_for_move(const std::vector<Move> &legal_moves);
    void inform_about_move(int player, const Move &move);
};

#endif
