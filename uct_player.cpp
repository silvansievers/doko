#include "uct_player.h"

#include "belief_game_state.h"
#include "options.h"
#include "uct.h"

#include <cassert>
#include <iostream>

using namespace std;

UctPlayer::UctPlayer(int player_number, const Options &options_) : Player(player_number), options(options_),
    current_belief_state(0), first_player(0), vorfuehrung(false), number_of_current_game(0), counter(0) {
    for (int i = 0; i < 4; ++i) {
        if (options.use_compulsory_solo())
            played_compulsory_solo[i] = false;
        else
            played_compulsory_solo[i] = true;
    }
}

UctPlayer::~UctPlayer() {
    delete current_belief_state;
}

void UctPlayer::check_vorfuehrung(int number_of_remaining_games) {
    int remaining_compulsory_solos = 0;
    for (int i = 0; i < 4; ++i) {
        if (!played_compulsory_solo[i])
            ++remaining_compulsory_solos;
    }
    assert(remaining_compulsory_solos <= number_of_remaining_games);
    if (remaining_compulsory_solos == number_of_remaining_games) {
        vorfuehrung = true;
    }
}

void UctPlayer::set_cards(Cards cards_) {
    Player::set_cards(cards_);
    delete current_belief_state;
    if (!vorfuehrung)
        check_vorfuehrung(options.get_number_of_games() - number_of_current_game);
    current_belief_state = new BeliefGameState(options, id, played_compulsory_solo, vorfuehrung, first_player, cards);
}

size_t UctPlayer::ask_for_move(const vector<Move> &legal_moves) {
    if (options.solo_disabled() && legal_moves.size() == 1 && legal_moves[0].is_game_type_move()) {
        return 0;
    }
    ++counter;
    int player_to_move = current_belief_state->get_player_to_move();
    assert(player_to_move == id);
    if (legal_moves.size() == 1) { // when there is only one option, do not bother to do a Uct computation
        // this should also catch the case when player has only one card left and thus no Uct-object should be created
        return 0;
    }
    if (options.get_announcement_option(id) == 0 && legal_moves[0].is_announcement_move()) { // if announcing is forbiden, no uct needed
        return 0;
    } else {
        Uct uct(options, *current_belief_state, cards, counter);
        if (options.get_uct_version(id) == 1) {
            const vector<BeliefGameState *> &compare_states = uct.get_belief_game_states();
            for (size_t i = 0; i < compare_states.size(); ++i) {
                vector<Move> legal_moves2;
                compare_states[i]->get_legal_moves(legal_moves2);
                assert(legal_moves.size() == legal_moves2.size());
                for (size_t i = 0; i < legal_moves.size(); ++i) {
                    assert(legal_moves[i] == legal_moves2[i]);
                }
            }
        }
        return uct.get_best_move();
    }
}

void UctPlayer::inform_about_move(int player, const Move &move) {
    Player::inform_about_move(player, move);
    //if (options.use_debug())
        //cout << "uctplayer " << id << " setting move" << endl;
    current_belief_state->set_move(player, move);
    //if (options.use_debug())
        //cout << endl;
}

void UctPlayer::inform_about_game_end(const int players_game_points[4]) {
    ++number_of_current_game;
    int new_points[4] = { 0, 0, 0, 0 };
    int new_points2[4] = { 0, 0, 0, 0 };
    // pass new_points2 as a dummy just to avoid output in GameState
    current_belief_state->get_score_points(new_points, new_points2);
    for (int i = 0; i < 4; ++i) {
        assert(players_game_points[i] == new_points[i]);
    }
    if (current_belief_state->is_compulsory_solo()) {
        int solo_player = current_belief_state->get_compulsory_solo_player();
        played_compulsory_solo[solo_player] = true;
    }
    if (!current_belief_state->is_compulsory_solo() || vorfuehrung)
        first_player = next_player(first_player);
}
