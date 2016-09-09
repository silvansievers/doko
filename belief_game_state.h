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

#ifndef BELIEF_GAME_STATE_H
#define BELIEF_GAME_STATE_H

#include "game_state.h"

#include <cassert>

/**
BeliefGameState is designed for the use by players, especially uct-players who need to keep track of the current game without having complete information, though. The mechanics of this class are somewhat complicated, although there are only two central methods: set_move() and get_legal_moves(). The functionallity of the two methods mirrors the one of ActualGameState with the only difference being that this class does not play the real game, but gets updated according to what happens in the real (or in a simulated) game. Thus set_move() needs to compute which player has to play next and what kind of move is next and needs to be able to return the legal moves for that player (which is done in get_legal_moves()). The latter is only needed for algorithms such as the uct-algorithm that simulates the game further on, i.e. this class is being needed for node-expansion in a game tree search.
*/

class BeliefGameState : public GameState {
private:
    const int player_number; // the player owning the instance of BeliefGameState
    bool played_compulsory_solo[4];
    int first_player;
    int player_to_move;
    Move next_move_type; // only the type matters
    Cards played_cards;
    int players_cards_count[4]; // number of cards for each player (this is used because cards is only set for the uct-player owning the gamestate instance but not for the other players, at least not until a rollout with a fixed card distribution is being computed)
    Cards cards_that_players_cannot_have[4];
    bool players_must_have_queen_of_clubs[4];
    bool uct_output;

    // fields needed for set_move() in order to "remember" which player was asked/played last time set_move() was called
    bool is_marriage; // to determine whether a GameTyp-move should only allow for a marriage or for all solos
    bool initialized; // true iff the vars in the "else"-block of set_move are initialized
    int players_left_to_ask_a_question;
    bool has_reservation[4];
    int reservation_count;
    int player_after_last_player_allowed_to_shorten;
    int player_to_ask_after_next_immediate_solo_move;
    bool no_player_with_reservation_and_open_compulsory_solo; // true if all players played their compulsory solo already
    int players_left_to_ask_about_is_solo_move;
    int first_positioned_player_for_a_lust_solo;
    int players_left_to_ask_about_announcement;
    int player_to_play_card; // next player to play a card

    void determine_first_move(bool vorfuehrung);
    void set_player_to_have_queen_of_clubs(int player, Card queen_of_clubs); // this method sets all other players than player to not have the given queen of clubs
    // when calling the methods set_xxx_move from set_move, "player" is always equal to "player_to_move", thus the extra argument is omitted
    void set_immediate_solo_move(const Move &move);
    void set_has_reservation_move(const Move &move);
    void set_is_solo_move(const Move &move);
    void set_game_type_move(const Move &move);
    void set_announcement_move(const Move &move);
    void set_card_move(const Move &move);

    // method related to heuristic move computation get_best_move_index()
    int play_valuable_card(const std::vector<Card> &possible_cards) const;
public:
    BeliefGameState(const Options &options, int player_number, const bool played_compulsory_solo[4],
                    bool vorfuehrung, int first_player, Cards players_cards);
    void set_other_players_cards(const Cards cards[4]);
    void set_move(int player, const Move &move);
    int get_player_to_move() const {
        return player_to_move;
    }
    void get_legal_moves(std::vector<Move> &legal_moves) const;
    Cards get_played_cards() const {
        return played_cards;
    }
    const Cards *get_cards_that_players_cannot_have() const {
        return cards_that_players_cannot_have;
    }
    const bool *get_players_must_have_queen_of_clubs() const {
        return players_must_have_queen_of_clubs;
    }
    const int *get_players_cards_count() const {
        return players_cards_count;
    }
    void set_uct_output(bool value) {
        uct_output = value;
    }
    bool operator==(const BeliefGameState &rhs) const {
        //const Options &options;
        for (int i = 0; i < 4; ++i) {
            assert(cards[i] == rhs.cards[i]);
            assert(players_team[i] == rhs.players_team[i]);
            assert(players_latest_moment_for_announcement[i] == rhs.players_latest_moment_for_announcement[i]);
            assert(players_known_team[i] == rhs.players_known_team[i]);
            assert(played_compulsory_solo[i] == rhs.played_compulsory_solo[i]);
            assert(players_cards_count[i] == rhs.players_cards_count[i]);
            assert(cards_that_players_cannot_have[i] == rhs.cards_that_players_cannot_have[i]);
            assert(players_must_have_queen_of_clubs[i] == rhs.players_must_have_queen_of_clubs[i]);
            assert(has_reservation[i] == rhs.has_reservation[i]);
        }
        for (int i = 0; i < 2; ++i) {
            assert(announcements[i] == rhs.announcements[i]);
            assert(first_announcement_in_time[i] == rhs.first_announcement_in_time[i]);
            assert(card_number_for_latest_possible_reply[i] == rhs.card_number_for_latest_possible_reply[i]);
        }
        assert(game_type == rhs.game_type);
        assert(tricks.size() == rhs.tricks.size());
        for (size_t i = 0; i < tricks.size(); ++i)
            assert(tricks[i] == rhs.tricks[i]);
        assert(session_instance == rhs.session_instance);
        assert(player_played_queen_of_clubs == rhs.player_played_queen_of_clubs);
        assert(teams_are_known == rhs.teams_are_known);
        assert(solo_or_marriage_player == rhs.solo_or_marriage_player);
        assert(compulsory_solo == rhs.compulsory_solo);
        assert(number_of_clarification_trick == rhs.number_of_clarification_trick);
        assert(player_number == rhs.player_number);
        assert(first_player == rhs.first_player);
        assert(player_to_move == rhs.player_to_move);
        assert(next_move_type == rhs.next_move_type);
        assert(played_cards == rhs.played_cards);
        assert(uct_output == rhs.uct_output);
        assert(is_marriage == rhs.is_marriage);
        assert(initialized == rhs.initialized);
        assert(players_left_to_ask_a_question == rhs.players_left_to_ask_a_question);
        assert(reservation_count == rhs.reservation_count);
        assert(player_after_last_player_allowed_to_shorten == rhs.player_after_last_player_allowed_to_shorten);
        assert(player_to_ask_after_next_immediate_solo_move == rhs.player_to_ask_after_next_immediate_solo_move);
        assert(no_player_with_reservation_and_open_compulsory_solo == rhs.no_player_with_reservation_and_open_compulsory_solo);
        assert(players_left_to_ask_about_is_solo_move == rhs.players_left_to_ask_about_is_solo_move);
        assert(first_positioned_player_for_a_lust_solo == rhs.first_positioned_player_for_a_lust_solo);
        assert(players_left_to_ask_about_announcement == rhs.players_left_to_ask_about_announcement);
        assert(player_to_play_card == rhs.player_to_play_card);
        return true;
    }
    int get_best_move_index(const std::vector<Move> &legal_moves) const;
};

#endif
