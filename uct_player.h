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
