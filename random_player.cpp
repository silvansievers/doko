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
