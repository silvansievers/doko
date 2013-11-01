#include "actual_game_state.h"

#include "game_type.h"
#include "options.h"
#include "player.h"

#include <iostream>

using namespace std;

ActualGameState::ActualGameState(const Options &options, Player *players_[4], int first_player,
                                 Cards cards_[4], const bool played_compulsory_solo_[4], bool vorfuehrung)
    : GameState(options, true) {
    for (int i = 0; i < 4; ++i) {
        cards[i] = cards_[i];
        players[i] = players_[i];
        played_compulsory_solo[i] = played_compulsory_solo_[i];
    }
    play(first_player, vorfuehrung);
}

void ActualGameState::print_trick() const {
    // print the last completed trick
    if (tricks.size() > 1) { // start printing when the first trick is completed, i.e. the second (emtpy) one has been inserted
        if (tricks.back().empty()) // if the most recent (i.e. current) trick is empty, dump the most recent completed one
            tricks[tricks.size() - 2].dump();
        if (game_finished()) // after the 12th trick, there is no new empty trick being inserted into tricks, therefore dump the last one
            tricks.back().dump();
    }
}

size_t ActualGameState::make_move(int player, const vector<Move> &move_options) const {
    size_t move_no = players[player]->ask_for_move(move_options);
    if (move_no < 0 || move_no >= move_options.size())
        players[player]->invalid_move();
    const Move &move = move_options[move_no];
    for (int i = 0; i < 4; ++i) {
        players[i]->inform_about_move(player, move);
    }
    return move_no;
}

bool ActualGameState::shorten_reservation_procedure(int player) {
    vector<Move> legal_immediate_solo_moves;
    legal_immediate_solo_moves.push_back(Move(IMMEDIATE_SOLO, false));
    legal_immediate_solo_moves.push_back(Move(IMMEDIATE_SOLO, true));
    size_t move_no = make_move(player, legal_immediate_solo_moves);
    if (options.use_verbose())
        cout << "player " << player;
    if (legal_immediate_solo_moves[move_no].get_answer()) {
        if (options.use_verbose())
            cout << " shortens reservation procedure and will announce a solo" << endl;
        ask_player_for_solo(player);
        return true;
    } else {
        if (options.use_verbose())
            cout << " does not announce an immediate solo" << endl;
    }
    return false;
}

void ActualGameState::ask_player_for_solo(int player) {
    vector<Move> legal_solos;
    legal_solos.push_back(Move(&diamonds_solo));
    legal_solos.push_back(Move(&hearts_solo));
    legal_solos.push_back(Move(&spades_solo));
    legal_solos.push_back(Move(&clubs_solo));
    legal_solos.push_back(Move(&jacks_solo));
    legal_solos.push_back(Move(&queens_solo));
    legal_solos.push_back(Move(&aces_solo));
    size_t move_no = make_move(player, legal_solos);
    game_type = legal_solos[move_no].get_game_type();
    solo_or_marriage_player = player;
    if (!played_compulsory_solo[player])
        compulsory_solo = true;
    assign_solo_player_to_re_team(player);
    if (compulsory_solo)
        cout << "compulsory solo: ";
    cout << "player " << player << " plays a " << *game_type << endl;
}

void ActualGameState::determine_game_type(int first_player, bool vorfuehrung) {
    // in the case of vorfuehrung, just ask the first player for his solo game
    if (vorfuehrung) {
        cout << "vorfuehrung: ";
        ask_player_for_solo(first_player);
        return;
    }

    // else: find the first player with open compulsory solo and ask him if he wants to announce an immediate solo
    int player = first_player; // iterate starting with first player
    for (int i = 0; i < 4; ++i) {
        if (!played_compulsory_solo[player])
            break;
        player = next_player(player);
    }
    // if all players played their compulsory solo, then player == first_player again and this player can announce an immediate solo
    int player_after_last_player_allowed_to_shorten = next_player(player); // store the player sitting behind the one who was most recently allowed to shorten the reservation procedure. as long as this player was not asked for a regular reservation and did not say "gesund" (i.e. no), nobody else may shorten the reservation procedure.
    if (shorten_reservation_procedure(player))
        return;

    // else: ask players for reservations in a regular manner, but still allow to shorten procedure
    bool reservations[4] = { false, false, false, false };
    int reservation_count = 0;
    bool no_player_with_reservation_and_open_compulsory_solo = true;
    vector<Move> legal_has_reservation_moves;
    legal_has_reservation_moves.push_back(Move(HAS_RESERVATION, false));
    legal_has_reservation_moves.push_back(Move(HAS_RESERVATION, true));
    player = first_player; // iterate starting with first player
    for (int i = 0; i < 4; ++i) {
        size_t move_no = make_move(player, legal_has_reservation_moves);
        if (options.use_verbose())
            cout << "player " << player;
        if (legal_has_reservation_moves[move_no].get_answer()) {
            if (options.use_verbose())
                cout << " has a reservation" << endl;
            reservations[player] = true;
            ++reservation_count;
            if (!played_compulsory_solo[player])
                no_player_with_reservation_and_open_compulsory_solo = false;
        } else {
            if (options.use_verbose())
                cout << " is gesund" << endl;
        }
        // do not ask players again after all 4 have been asked (i.e. in iteration 3)
        if (i != 3) {
            player = next_player(player);
            if (reservation_count == 0 && player_after_last_player_allowed_to_shorten == player) { // only if nobody has a reservation so far and after the player who was last allowed to shorten the reservation procedure said "gesund" need to check who may shorten next
                int player_it = player;
                // iterate over the remaining 4-i-1 players and allow the first one with open compulsory solo (or the first one if none has an open compulsory solo left) to shorten the reservation procedure
                for (int j = 0; j < 4 - i - 1; ++j) {
                    if (!played_compulsory_solo[player_it])
                        break;
                    player_it = next_player(player_it);
                }
                if (player_it != first_player) { // this means the inner loop did not run through entirely, i.e. a player with open compulsory solo who now may shorten the reservation procedure was found
                    if (shorten_reservation_procedure(player_it))
                        return;
                    else
                        player_after_last_player_allowed_to_shorten = next_player(player_it);
                }
                else { // no open compulsory solo remaining, ask player for shorten reservation procedure
                    if (shorten_reservation_procedure(player))
                        return;
                    else
                        player_after_last_player_allowed_to_shorten = next_player(player);
                }
            }
        }
    }

    // no reservations: play regular game (which may be a secret solo, of course)
    if (reservation_count == 0) {
        cout << "regular game will be played" << endl;
        game_type = &regular;
        for (int i = 0; i < 4; ++i) {
            if (cards[i].contains_card(CQ) || cards[i].contains_card(CQ_))
                players_team[i] = 1;
            else
                players_team[i] = 0;
        }
        return;
    }

    // else: ask players with reservation if they want to play a solo or not
    // if there is an open compulsory solo left, ask all players with a reservation if they want to play a solo or not and stop as soon as a player with an open compulsory solo answers "yes". if this does not determine the soloist (because the only player with an open compulsory solo who announced a reservation wants to play a marriage), then the first player who wants to play a lust solo is admitted. if there is anyway no open compulsory solo left, then admit the first player who wants to play a solo.
    // NOTE: we do not bother again to allow players to immediately announce their solo as soon as they know that somebody sitting before them (or behind them but with the higher priority because they have an open compulsory solo) does not want to play a solo (but a marriage).
    player = first_player;
    int first_positioned_player_for_a_lust_solo = -1;
    for (int i = 0; i < 4; ++i) {
        if (reservations[player]) {
            vector<Move> legal_is_solo_moves;
            if (cards[player].contains_card(CQ) && cards[player].contains_card(CQ_)) // only can say "no" if have both queens of clubs
                legal_is_solo_moves.push_back(Move(IS_SOLO, false));
            legal_is_solo_moves.push_back(Move(IS_SOLO, true));
            size_t move_no = make_move(player, legal_is_solo_moves);
            if (options.use_verbose())
                cout << "player " << player;
            if (legal_is_solo_moves[move_no].get_answer()) {
                if (options.use_verbose())
                    cout << " wants to play a solo" << endl;
                if (reservation_count == 1 || !played_compulsory_solo[player] || no_player_with_reservation_and_open_compulsory_solo) {
                    // admit the only player who wants to play solo, first player with a compulsory solo or the first player who wants to play a lust solo if there is no player with open compulsory solo who has a reservation
                    // NOTE: the check for reservation_count == 1 is actually not necessary because if the player is not being asked right here, he still will be asked after this loop has finished (as there are no more cases for which "reservations" is true)
                    ask_player_for_solo(player);
                    return;
                } else { // first_positioned_player_for_a_lust_solo is the first positioned player who wants to play a lust solo (to avoid another loop over all players checking for lust solos after finishing this loop without having a player play his compulsory solo)
                    if (first_positioned_player_for_a_lust_solo == -1)
                        first_positioned_player_for_a_lust_solo = player;
                }
            } else { // no solo reservation => must be a marriage reservation!
                if (options.use_verbose())
                    cout << " does not want to play a solo" << endl;
                if (reservation_count == 1) { // marriage is only allowed if nobody else has a reservation
                    vector<Move> legal_game_type_moves;
                    legal_game_type_moves.push_back(Move(&marriage));
                    size_t move_no = make_move(player, legal_game_type_moves);
                    assert(move_no == 0);
                    cout << "player " << player << " has a marriage" << endl;
                    game_type = &marriage;
                    solo_or_marriage_player = player;
                    players_team[player] = 1;
                    players_known_team[player] = 1;
                    int player_it = next_player(player);
                    for (int i = 0; i < 3; ++i) {
                        players_team[player_it] = 0;
                        players_known_team[player_it] = 0; // also initialize the known teams such that if the marriage fails, nothing needs to be updated, and if it succeeds, there is just one player to be updated (both players_team and players_known_team)
                        player_it = next_player(player_it);
                    }
                    return;
                }
            }
        }
        player = next_player(player);
    }
    // function did not return yet => let player_to_play_solo play his lust solo
    assert(game_type == 0);
    assert(first_positioned_player_for_a_lust_solo != -1);
    ask_player_for_solo(first_positioned_player_for_a_lust_solo);
    assert(!compulsory_solo);
}

void ActualGameState::play(int first_player, bool vorfuehrung) {
    determine_game_type(first_player, vorfuehrung);
    int players_left_to_ask_about_announcement = 4;
    if (compulsory_solo)
        first_player = solo_or_marriage_player;
    tricks.push_back(Trick(game_type, first_player));
    int player_to_ask = first_player;
    int current_player = first_player;
    while (!game_finished()) {
        if (players_left_to_ask_about_announcement != 0) {
            assert(player_to_ask != -1);
            bool ann_poss = announcement_possible(player_to_ask, cards[player_to_ask].size());
            if (ann_poss) {
                vector<Move> legal_announcements;
                get_legal_announcements_for_player(player_to_ask, legal_announcements);
                size_t move_no = make_move(player_to_ask, legal_announcements);
                announcement_t announcement = legal_announcements[move_no].get_announcement();
                if (announcement != NONE) {
                    if (options.use_verbose())
                        cout << "player " << player_to_ask << " belonging to team " << (players_team[player_to_ask] == 1 ? "re" : "kontra") << " announced: " << announcement << endl;
                    if (options.get_announcing_version() == 0 || announcement == SCHWARZ || (announcement == REKON && corrected_number_of_cards(cards[player_to_ask].size()) < 11))
                        players_left_to_ask_about_announcement = 3; // do not ask the same player again if he announced black or did a reply
                    else
                        players_left_to_ask_about_announcement = 4; // ask the same player again
                    bool is_re_player = legal_announcements[move_no].get_re_team();
                    assert(players_team[player_to_ask] == is_re_player);
                    set_announcement(player_to_ask, announcement, is_re_player, cards[player_to_ask].size());
                } else {
                    if (options.use_verbose())
                        cout << "player " << player_to_ask << " does not announce anything" << endl;
                    --players_left_to_ask_about_announcement;
                }
            } else {
                --players_left_to_ask_about_announcement;
            }
            if (players_left_to_ask_about_announcement > 0) {
                if (players_left_to_ask_about_announcement != 4) // players_left_to_ask_about_announcement can only be set to four if the announcing version used is 1 and if a player just did an announcement and thus he should be asked again. otherwise the next player should be asked, no matter which announcing version is being used
                    player_to_ask = next_player(player_to_ask);
            } else
                player_to_ask = -1;
        } else {
            vector<Move> legal_cards;
            tricks.back().get_legal_cards_for_player(current_player, legal_cards, cards[current_player]);
            size_t move_no = make_move(current_player, legal_cards);
            Card card = legal_cards[move_no].get_card();
            if (options.use_verbose())
                cout << "player " << current_player << " played card: " << card << endl;
            player_to_ask = next_player(current_player); // next player should be asked for an announcement
            current_player = update(current_player, card);
            //if (cards[0].size() == 10)
                //assert(false);
            players_left_to_ask_about_announcement = 3;
            if (options.use_verbose())
                print_trick();
        }
    }
    cout << "game finished" << endl << endl;
}