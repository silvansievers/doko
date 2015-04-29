#include "belief_game_state.h"

#include "game_type.h"
#include "options.h"

#include <iostream>
#include <set>

using namespace std;

BeliefGameState::BeliefGameState(const Options &options, int player_number_, const bool played_compulsory_solo_[4],
                                 bool vorfuehrung, int first_player_, Cards players_cards)
    : GameState(options, false), player_number(player_number_), first_player(first_player_),
    player_to_move(first_player_), uct_output(true), is_marriage(false), initialized(false) {
    for (int i = 0; i < 4; ++i) {
        if (i == player_number)
            cards[i] = players_cards;
        else
            cards[i] = Cards();
        played_compulsory_solo[i] = played_compulsory_solo_[i];
        players_cards_count[i] = 12;
        cards_that_players_cannot_have[i] = Cards();
        players_must_have_queen_of_clubs[i] = false;
    }
    // next_move_type remains uninitialized!
    // played_cards is not explicitely set to Cards() but should have this value by default

    // initializiation of variables needed in set_move
    players_left_to_ask_a_question = 4;
    for (int i = 0; i < 4; ++i)
        has_reservation[i] = false;
    reservation_count = 0;
    player_to_ask_after_next_immediate_solo_move = first_player;
    no_player_with_reservation_and_open_compulsory_solo = true;
    players_left_to_ask_about_is_solo_move = 0;
    first_positioned_player_for_a_lust_solo = -1;

    determine_first_move(vorfuehrung);
}

void BeliefGameState::determine_first_move(bool vorfuehrung) {
    if (options.solo_disabled()) {
        // if no solos are allowed, we assume that the game type is regular and
        // the first move is an announcement move. We capture the case of a
        // marriage in set_move().
        game_type = &regular;
        assert(tricks.empty());
        tricks.push_back(Trick(game_type, player_to_move)); // initialize tricks already here that get_legal_moves works also for an uct player who is starting the card play (thus setting it when playing the first card would be too late)
        next_move_type = Move(NONE, false);
        if (options.use_debug() && uct_output)
            cout << "regular game will be played" << endl;
        if (options.use_debug() && uct_output)
            cout << "next move will be: announcement move" << endl;
    } else if (vorfuehrung) {
        next_move_type = Move(&regular);
    } else {
        int player_it = player_to_move;
        // test for open compulsory solos: first player can shorten reservation procedure
        for (int i = 0; i < 4; ++i) {
            if (!played_compulsory_solo[player_it]) {
                player_to_move = player_it;
                player_after_last_player_allowed_to_shorten = next_player(player_to_move);
                next_move_type = Move(IMMEDIATE_SOLO, false);
                return;
            }
            player_it = next_player(player_it);
        }
        // no open compulsory solos left: first player can shorten reservation procedure (player_to_move already correctly set!)
        next_move_type = Move(IMMEDIATE_SOLO, false);
        assert(player_to_move == first_player);
        player_after_last_player_allowed_to_shorten = next_player(player_to_move);
    }
}

void BeliefGameState::set_player_to_have_queen_of_clubs(int player, Card queen_of_clubs) {
    //if (options.use_debug() && uct_output)
        //cout << "set player " << player << " to have specific queen of clubs" << endl;
    for (int i = 0; i < 4; ++i) {
        if (i == player)
            continue;
        cards_that_players_cannot_have[i].add_card(queen_of_clubs);
    }
}

void BeliefGameState::set_other_players_cards(const Cards cards_[4]) {
    for (int i = 0; i < 4; ++i) {
        if (i == player_number) {
            assert(cards_[i].empty());
            continue;
        }
        cards[i] = cards_[i];
    }
    // when assigning cards to players in a rollout, set the teams if the game type is either not set (because if the game type will be a regular game then, there is no way that the teams could be set by then) or if it is already set to regular. in all other cases this is done before the game (or automatically in case of a marriage)
    if (!teams_are_known && (game_type == 0 || *game_type == regular)) {
        for (int i = 0; i < 4; ++i) {
            if (players_known_team[i] == -1) {
                if (cards[i].contains_card(CQ) || cards[i].contains_card(CQ_))
                    players_team[i] = 1;
                else
                    players_team[i] = 0;
            } else
                assert(players_known_team[i] == players_team[i]);
        }
    }
}

void BeliefGameState::set_immediate_solo_move(const Move &move) {
    //if (options.use_debug() && uct_output)
        //cout << "set move: move is immediate solo move" << endl;
    // player announces an immediate solo
    if (move.get_answer()) {
        if (options.use_debug() && uct_output)
            cout << " shortens reservation procedure and will announce a solo" << endl;
        next_move_type = Move(&regular);
        // no need to update player_to_move
        if (options.use_debug() && uct_output)
            cout << "next move will be: game type move" << endl;
        return;
    }

    // else: player does not announce an immediate solo
    if (options.use_debug() && uct_output)
        cout << " does not announce an immediate solo" << endl;
    next_move_type = Move(HAS_RESERVATION, false);
    player_after_last_player_allowed_to_shorten = next_player(player_to_move);
    if (players_left_to_ask_a_question == 4) {
        // this is only true for the first time a player is asked for an immediate solo, from then on, starting with the first_player, a player is first asked for a regular reservation, and if he denies, then there might be another player who is asked for an immediate solo, in which case players_left_to_ask_a_question must be smaller than 4 and in which case, if the player denies, he is immediately asked if he has a regular reservation
        player_to_move = first_player;
        players_left_to_ask_a_question = 3;
    } else if (player_to_ask_after_next_immediate_solo_move != player_to_move) {
        // go back to the next player who needs to be asekd for a regular reservation
        player_to_move = player_to_ask_after_next_immediate_solo_move;
    }
    if (options.use_debug() && uct_output)
        cout << "next move will be: has reservation move" << endl;
}

void BeliefGameState::set_has_reservation_move(const Move &move) {
    //if (options.use_debug() && uct_output)
        //cout << "set move: move is has reservation move" << endl;
    if (move.get_answer()) {
        if (options.use_debug() && uct_output)
            cout << " has a reservation" << endl;
        has_reservation[player_to_move] = true;
        ++reservation_count;
        if (!played_compulsory_solo[player_to_move])
            no_player_with_reservation_and_open_compulsory_solo = false;
        ++players_left_to_ask_about_is_solo_move;
    } else {
        if (options.use_debug() && uct_output)
            cout << " is gesund" << endl;
    }
    player_to_move = next_player(player_to_move);
    if (players_left_to_ask_a_question > 0) {
        --players_left_to_ask_a_question;
        if (reservation_count == 0 && player_to_move == player_after_last_player_allowed_to_shorten) {
            // no reservation so far and the first player after the last player who was asked to shorten the reservation procedure is going to be the next player_to_move - but only if there is not a player who can now shorten the reservation procedure, knowing that the previous player just said "gesund"
            int player_it = player_to_move;
            // iterate over the remaining players to ask (+1 because the number was already decreased a few lines above) to find another player who may now shorten the reservation procedure (if not, then continue with the next positioned one)
            for (int j = 0; j < players_left_to_ask_a_question + 1; ++j) {
                if (!played_compulsory_solo[player_it])
                    break;
                player_it = next_player(player_it);
            }
            next_move_type = Move(IMMEDIATE_SOLO, false);
            player_to_ask_after_next_immediate_solo_move = player_to_move; // in case of the next if-clause evaluating to true and thus some players are "skipped" because there is a player who is being asked for an immediate solo, after that question is answered with no, need to go back to the current player who has to be asked for a reservation
            if (player_it != first_player) // found a player who is now allowed to shorten the reservation procedure
                player_to_move = player_it;
            // else: no need to update player_to_move as the player who just denied an immediate solo is going to be asked for a regular reservation
            if (options.use_debug() && uct_output)
                cout << "next move will be: immediate solo move" << endl;
        } else {
            // no need to update next_move_type
            // player_to_move already updated
            if (options.use_debug() && uct_output)
                cout << "next move will be: has reservation move" << endl;
        }
        return;
    }

    // else: all players have been asked for a reservation, thus player_to_move should (for the moment) be set to first_player
    assert(player_to_move == first_player);
    // there is at least one player having a reservation, ask him if he wants to play a solo
    if (reservation_count > 0) {
        next_move_type = Move(IS_SOLO, false);
        // find first player with reservation
        while (!has_reservation[player_to_move]) {
            player_to_move = next_player(player_to_move);
        }
        --players_left_to_ask_about_is_solo_move; // first player is going to be asked next
        if (options.use_debug() && uct_output)
            cout << "next move will be: is solo move" << endl;
        return;
    }

    // else: no reservations, thus play regular game
    game_type = &regular;
    assert(tricks.empty());
    tricks.push_back(Trick(game_type, player_to_move)); // initialize tricks already here that get_legal_moves works also for an uct player who is starting the card play (thus setting it when playing the first card would be too late)
    next_move_type = Move(NONE, false);
    if (options.use_debug() && uct_output)
        cout << "regular game will be played" << endl;
    if (options.use_debug() && uct_output)
        cout << "next move will be: announcement move" << endl;
}

void BeliefGameState::set_is_solo_move(const Move &move) {
    //if (options.use_debug() && uct_output)
    //cout << "set move: move is is solo move" << endl;
    // player wants to play a solo
    if (move.get_answer()) {
        if (options.use_debug() && uct_output)
            cout << " wants to play a solo" << endl;
        // admit a player to play solo if there is only one reservation, if he did not play a compulsory solo (i.e. he is the first positioned player with an open compulsory solo and thus has highest priority) or if there is no player with open compulsory solo and a reservation (i.e. he is the first positioned player to play a solo and nobody else can have higher priority than he does)
        if (reservation_count == 1 || !played_compulsory_solo[player_to_move] || no_player_with_reservation_and_open_compulsory_solo) {
            next_move_type = Move(&regular);
            // no need to update player_to_move
            if (options.use_debug() && uct_output)
                cout << "next move will be: game type move" << endl;
            return;
        }

        // else: player could not get admitted to play his solo
        if (first_positioned_player_for_a_lust_solo == -1) // store the first player who wants to play a solo but could not get admitted immediately (e.g. because another player with open compulsory solo sitting behind him also has a reservation)
            first_positioned_player_for_a_lust_solo = player_to_move;
    }

    // player does not want to play a solo
    if (!move.get_answer()) {
        if (options.use_debug() && uct_output)
            cout << " does not want to play a solo" << endl;
        // answering no implies the player wants to play a marriage
        set_player_to_have_queen_of_clubs(player_to_move, CQ);
        set_player_to_have_queen_of_clubs(player_to_move, CQ_);
        if (reservation_count == 1) { // allow marriage
            next_move_type = Move(&regular);
            // no need to update player_to_move
            is_marriage = true;
            if (options.use_debug() && uct_output)
                cout << "next move will be: game type move" << endl;
            return;
        }
    }

    // else: either the player answered yes, but was not allowed to immediately play announce his solo because he did not have the highest priority or the player did not want to play a solo but a marriage, but there was more than one reservation and thus he was not allowed to play it. In both cases, need to ask the remaining players with a reservation if existent or need to admit the first player positioned for a lust solo (which may be the one who just said yes but was not ommited or a player who has been asked earlier already)
    // there are still players with reservation that need to be asked if they want to play solo or not
    if (players_left_to_ask_about_is_solo_move > 0) {
        // no need to update next_move_type
        --players_left_to_ask_about_is_solo_move; // next player is going to be asked
        player_to_move = next_player(player_to_move);
        while (!has_reservation[player_to_move]) {
            player_to_move = next_player(player_to_move);
        }
        if (options.use_debug() && uct_output)
            cout << "next move will be: is solo move" << endl;
        return;
    }

    // else: no more players need to be asked, thus allow the first positioned player with a lust solo to play
    next_move_type = Move(&regular);
    assert(first_positioned_player_for_a_lust_solo != -1);
    player_to_move = first_positioned_player_for_a_lust_solo;
    if (options.use_debug() && uct_output)
        cout << "next move will be: game type move" << endl;
}

void BeliefGameState::set_game_type_move(const Move &move) {
    //if (options.use_debug() && uct_output)
        //cout << "set move: move is game type move" << endl;
    assert(game_type == 0);
    game_type = move.get_game_type();
    solo_or_marriage_player = player_to_move;
    // player_to_move plays a solo
    if (*game_type != marriage) {
        if (options.use_debug() && uct_output)
            cout << " plays a " << *game_type << endl;
        assign_solo_player_to_re_team(player_to_move);
        next_move_type = Move(NONE, false);
        if (!played_compulsory_solo[player_to_move]) {
            compulsory_solo = true;
            // already set! player_to_move = solo_or_marriage_player;
        } else {
            player_to_move = first_player;
        }
        assert(announcement_possible(player_to_move, players_cards_count[player_to_move]));
        if (options.use_debug() && uct_output)
            cout << "next move will be: announcement move" << endl;
        return;
    }

    // else: player_to_move has a marriage and thus both queens of clubs. obviously the other players then cannot have any queen of clubs
    set_player_to_have_queen_of_clubs(player_to_move, CQ);
    set_player_to_have_queen_of_clubs(player_to_move, CQ_);
    players_team[player_to_move] = 1;
    players_known_team[player_to_move] = 1;
    int player_it = next_player(player_to_move);
    for (int i = 0; i < 3; ++i) {
        players_team[player_it] = 0;
        players_known_team[player_it] = 0; // also initialize the known teams such that if the marriage fails, nothing needs to be updated, and if it succeeds, there is just one player to be updated (both players_team and players_known_team)
        player_it = next_player(player_it);
    }
    next_move_type = Move(no_card);
    player_to_move = first_player;
    if (options.use_debug() && uct_output)
        cout << " has a marriage" << endl;
    if (options.use_debug() && uct_output)
        cout << "next move will be: card move" << endl;
}

void BeliefGameState::set_announcement_move(const Move &move) {
    //if (options.use_debug() && uct_output)
        //cout << "set move: move is announcement move" << endl;
    announcement_t announcement = move.get_announcement();

    // player is doing an announcement
    if (announcement != NONE) {
        bool is_re_player = move.get_re_team();
        if (options.use_debug() && uct_output)
            cout << " belonging to team " << (is_re_player ? "re" : "kontra") << " announced: " << announcement << endl;

        // set cards which players cannot have / must have (queens of clubs) (avoid setting this repeatedly because when a player finally played a queen of clubs, players_must_have_queen_of_clubs is set false for him and could become true again which causes problems to card assignment)
        if (*game_type == regular && player_to_move != player_number && players_known_team[player_to_move] == -1) {
            // if player_number has a queen of clubs, then determine the other queen of clubs
            if (cards[player_number].contains_card(CQ) || cards[player_number].contains_card(CQ_)) {
                Card queen_of_clubs;
                if (cards[player_number].contains_card(CQ))
                    queen_of_clubs = CQ_;
                else
                    queen_of_clubs = CQ;
                if (is_re_player) { // player_to_move belongs to re, thus the other two players cannot have queen_of_clubs
                    set_player_to_have_queen_of_clubs(player_to_move, queen_of_clubs);
                } else { // player_to_move belongs to kontra, thus he cannot have queen_of_clubs
                    // of course he can neither have the other queen of clubs, but this is included already in the fact that he cannot have any of the cards player_number owns
                    //if (options.use_debug() && uct_output)
                        //cout << "set player " << player_to_move << " not to have a queen of clubs" << endl;
                    cards_that_players_cannot_have[player_to_move].add_card(queen_of_clubs);
                }
            } else { // player_number is kontra player
                if (!played_cards.contains_card(CQ) && !played_cards.contains_card(CQ_)) { // no queen of clubs has been played so far
                    if (is_re_player) {
                        // cannot infer which queen of clubs a player has when announcing re. deciding for an arbitrary card of them could cause inconsistencies later when the player plays the other card instead of the chosen one (in which case one would need to change cards_that_players_cannot_have etc which seems to be more complicated than have an extra field players_must_have_queen_of_clubs and let uct decide which card to assign whom).
                        //if (options.use_debug() && uct_output)
                            //cout << "set player " << player_to_move << " to have some queen of clubs" << endl;
                        players_must_have_queen_of_clubs[player_to_move] = true;
                    } else {
                        //if (options.use_debug() && uct_output)
                            //cout << "set player " << player_to_move << " not to have a queen of clubs" << endl;
                        cards_that_players_cannot_have[player_to_move].add_card(CQ);
                        cards_that_players_cannot_have[player_to_move].add_card(CQ_);
                        // NOTE: Cannot infer that the two other players (not player_number nor the announcing player who both are kontra) must have queen of clubs because one of them might play a silenct solo.
                    }
                } else if (!(played_cards.contains_card(CQ) && played_cards.contains_card(CQ_))) {
                    // exactly one queen of clubs has been played
                    Card queen_of_clubs;
                    if (played_cards.contains_card(CQ))
                        queen_of_clubs = CQ_;
                    else if (played_cards.contains_card(CQ_))
                        queen_of_clubs = CQ;
                    else
                        assert(false);
                    if (is_re_player) { // announcing player has queen_of_clubs
                        set_player_to_have_queen_of_clubs(player_to_move, queen_of_clubs);
                    } else { // announcing player cannot have queen_of_clubs
                        //if (options.use_debug() && uct_output)
                            //cout << "set player " << player_to_move << " not to have a queen of clubs" << endl;
                        cards_that_players_cannot_have[player_to_move].add_card(queen_of_clubs);
                        // NOTE: cannot infer that the remaining player (not player_number or the announcing player who both are kontra, nor the player who played the first queen of clubs before) must have the other queen of clubs, because the player who played the first queen of clubs might play a silent solo (thus has both queens of clubs)
                    }
                }
                // else: both queens of clubs have been played, do not update anything
            }
        }
        // NOTE: set_announcment must be called after the above block which updates the cards a player needs to have, because this is otherwise in an incorrect way (because it would then use the already updated information about the announcing player)
        set_announcement(player_to_move, announcement, is_re_player, players_cards_count[player_to_move]);
        if (options.get_announcing_version() == 0 || announcement == SCHWARZ || (announcement == REKON && corrected_number_of_cards(players_cards_count[player_to_move]) < 11))
            players_left_to_ask_about_announcement = 3; // do not ask the same player again if he announced black or did a reply
        else
            players_left_to_ask_about_announcement = 4; // ask the same player again
    }

    // player does not do an announcement, thus just finish asking the remaining players
    if (announcement == NONE) {
        if (options.use_debug() && uct_output)
            cout << " does not announce anything" << endl;
    }

    if (players_left_to_ask_about_announcement != 4) // players_left_to_ask_about_announcement can only be set to four if the announcing version used is 1 and if a player just did an announcement and thus he should be asked again. otherwise the next player should be asked, no matter which announcing version is being used
        player_to_move = next_player(player_to_move);
}

void BeliefGameState::set_card_move(const Move &move) {
    assert(player_to_play_card == player_to_move);
    //if (options.use_debug() && uct_output)
        //cout << "set move: move is card move" << endl;
    --players_cards_count[player_to_move];
    Card card = move.get_card();
    if (options.use_debug() && uct_output)
        cout << " played card " << card << endl;
    // for a regular game and if a queen of clubs was played, update cards_that_players_cannot_have
    if (*game_type == regular && (card == CQ || card == CQ_)) {
        if (players_must_have_queen_of_clubs[player_to_move])
            players_must_have_queen_of_clubs[player_to_move] = false;
        // check if there is a player who must have a queen of clubs which can now be determined as the other queen of clubs different from card
        int player_it = next_player(player_to_move);
        for (int i = 0; i < 3; ++i) {
            if (player_it != player_number && players_must_have_queen_of_clubs[player_it]) {
                players_must_have_queen_of_clubs[player_it] = false;
                Card queen_of_clubs;
                if (card == CQ)
                    queen_of_clubs = CQ_;
                else
                    queen_of_clubs = CQ;
                set_player_to_have_queen_of_clubs(player_it, queen_of_clubs);
            }
            player_it = next_player(player_it);
        }
    }
    // this is a trick (or HACK) to make "update" work (which removes "card" from "cards" again and thus needs card to be included in cards)
    if (!cards[player_to_move].contains_card(card))
        cards[player_to_move].add_card(card);
    played_cards.add_card(card);
    Cards trick_suit = tricks.back().get_trick_suit();
    if (!trick_suit.contains_card(card))
        cards_that_players_cannot_have[player_to_move].add_cards(trick_suit);
    player_to_move = next_player(player_to_move); // next player should be asked for an announcement
    player_to_play_card = update(player_to_play_card, card);
    players_left_to_ask_about_announcement = 3;
}

void BeliefGameState::set_move(int player, const Move &move) {
    if (options.solo_disabled() && move.is_game_type_move()) {
        // undo regular game assumption
        game_type = 0;
        tricks.clear();
        player_to_move = player;
        next_move_type = Move(&regular);
    }
    if (player != player_to_move)
        cout << player << " " << player_to_move << endl;
    assert(player == player_to_move);
    if (move.is_question_move() != next_move_type.is_question_move() || move.is_game_type_move() != next_move_type.is_game_type_move()
        || move.is_announcement_move() != next_move_type.is_announcement_move() || move.is_card_move() != next_move_type.is_card_move()) {
        cout << move.is_question_move() << " " << next_move_type.is_question_move() << endl;
        cout << move.is_game_type_move() << " " << next_move_type.is_game_type_move() << endl;
        cout << move.is_announcement_move() << " " << next_move_type.is_announcement_move() << endl;
        cout << move.is_card_move() << " " << next_move_type.is_card_move() << endl;
    }
    assert(move.is_question_move() == next_move_type.is_question_move());
    assert(move.is_game_type_move() == next_move_type.is_game_type_move());
    assert(move.is_announcement_move() == next_move_type.is_announcement_move());
    assert(move.is_card_move() == next_move_type.is_card_move());
    if (options.use_debug() && uct_output)
        cout << "(uct instance) player " << player;
    if (move.is_question_move()) {
        switch (move.get_question_type()) {
            case IMMEDIATE_SOLO:
                set_immediate_solo_move(move);
                return;
            case HAS_RESERVATION:
                set_has_reservation_move(move);
                return;
            case IS_SOLO:
                set_is_solo_move(move);
                return;
        }
    }
    if (move.is_game_type_move()) {
        set_game_type_move(move);
        // TODO: the following two lines also exist in set_has_reservation_move()
        assert(tricks.empty());
        tricks.push_back(Trick(game_type, player_to_move)); // initialize tricks already here such that get_legal_moves works also for an uct player who is starting the card play (thus setting it when playing the first card would be too late)
        return;
    }

    if (!initialized) {
        initialized = true;
        players_left_to_ask_about_announcement = 3; // when first reaching this part, the current move should be an announcement move, thus there are 3 remaining players to be asked. exception: marriage. then this part is reached with a card move, in which case no announcement is possible until the clarification trick; if an announcement is possible, players_left_to_ask_about_announcement = 3 would need to be set anyway.
        player_to_play_card = player_to_move;
    }

    if (move.is_announcement_move()) {
        set_announcement_move(move);
    } else if (move.is_card_move()) {
        set_card_move(move);
    }

    // for both cases of an announcement move and a card move, the next move will be determined in the same way: either there are still players to be asked for an announcement and these are asked if they are allowed to announce, or the next move needs to be a card move.
    while (players_left_to_ask_about_announcement > 0) {
        --players_left_to_ask_about_announcement;
        bool ann_poss = announcement_possible(player_to_move, players_cards_count[player_to_move]);
        if (ann_poss) {
            next_move_type = Move(NONE, false);
            if (options.use_debug() && uct_output)
                cout << "next move will be: announcement move" << endl;
            return;
        }
        player_to_move = next_player(player_to_move);
    }
    assert(players_left_to_ask_about_announcement == 0); // all players have been asked for an announcement (or no player is allowed to do any further announcements), thus the next move is a card move
    next_move_type = Move(no_card);
    player_to_move = player_to_play_card;
    if (options.use_debug() && uct_output)
        cout << "next move will be: card move" << endl;
}

void BeliefGameState::get_legal_moves(vector<Move> &legal_moves) const {
    if (!(!cards[0].empty() || !cards[1].empty() || !cards[2].empty() || !cards[3].empty())) {
        cout << "player 0: " << cards[0] << endl;
        cout << "player 1: " << cards[1] << endl;
        cout << "player 2: " << cards[2] << endl;
        cout << "player 3: " << cards[3] << endl;
    }
    assert(!cards[0].empty() || !cards[1].empty() || !cards[2].empty() || !cards[3].empty());
    if (next_move_type.is_question_move()) {
        switch (next_move_type.get_question_type()) {
            case IMMEDIATE_SOLO:
                legal_moves.push_back(Move(IMMEDIATE_SOLO, false));
                legal_moves.push_back(Move(IMMEDIATE_SOLO, true));
                break;
            case HAS_RESERVATION:
                legal_moves.push_back(Move(HAS_RESERVATION, false));
                legal_moves.push_back(Move(HAS_RESERVATION, true));
                break;
            case IS_SOLO:
                if (cards[player_to_move].contains_card(CQ) && cards[player_to_move].contains_card(CQ_)) // only can say "no" if have both queens of clubs
                    legal_moves.push_back(Move(IS_SOLO, false));
                legal_moves.push_back(Move(IS_SOLO, true));
                break;
        }
    } else if (next_move_type.is_game_type_move()) {
        if (is_marriage)
            legal_moves.push_back(Move(&marriage));
        else {
            legal_moves.push_back(Move(&diamonds_solo));
            legal_moves.push_back(Move(&hearts_solo));
            legal_moves.push_back(Move(&spades_solo));
            legal_moves.push_back(Move(&clubs_solo));
            legal_moves.push_back(Move(&jacks_solo));
            legal_moves.push_back(Move(&queens_solo));
            legal_moves.push_back(Move(&aces_solo));
        }
    } else if (next_move_type.is_announcement_move()) {
        assert(announcement_possible(player_to_move, players_cards_count[player_to_move]));
        get_legal_announcements_for_player(player_to_move, legal_moves);
    } else if (next_move_type.is_card_move()) {
        tricks.back().get_legal_cards_for_player(player_to_move, legal_moves, cards[player_to_move]);
    }
}

int BeliefGameState::play_valuable_card(const vector<Card> &possible_cards) const {
    int value_to_check = 11;
    while (true) {
        // iterate over all cards searching for a card worth value_to_check points. if not successful, decrease value_to_check and repeat.
        for (size_t i = 0; i < possible_cards.size(); ++i) {
            if (possible_cards[i].get_value() == value_to_check)
                return i;
        }
        if (value_to_check == 11 || value_to_check == 4 || value_to_check == 3)
            --value_to_check;
        else if (value_to_check == 10)
            value_to_check = 4;
        else if (value_to_check == 2)
            value_to_check = 0;
        else
            assert(false);
    }
}

int BeliefGameState::get_best_move_index(const vector<Move> &legal_moves) const {
    assert(legal_moves[0].is_card_move());
    /*cout << "calling get best move index, player to move: " << player_to_move << endl;
    for (int i = 0; i < 4; ++i) {
        cout << "player " << i << "'s team: " << players_team[i] << endl;
    }
    cout << "legal cards to play: " << endl;
    for (size_t i = 0; i < legal_moves.size(); ++i) {
        assert(legal_moves[i].is_card_move());
        //cout << legal_moves[i].get_card();
    }
    cout << endl;
    cout << "other players cards: " << endl;
    int playerit = next_player(player_to_move);
    for (int i = 0; i < 3; ++i) {
        cout << "player " << playerit << " " << cards[playerit] << endl;
        playerit = next_player(playerit);
    }*/

    int trick_size = tricks.back().get_size();
    if (!tricks.back().empty()) {
        assert(trick_size < 4);
        int trick_taken_by = tricks.back().taken_by();
        //cout << "trick taken by so far: " << trick_taken_by << endl;
        if (players_team[trick_taken_by] == players_team[player_to_move]) {
            // trick is owned by a teammate so far, check if a player of the other team can play a higher card
            Trick current_trick(tricks.back());
            current_trick.set_card(player_to_move, no_card);
            int player_it = next_player(player_to_move);
            for (int i = 0; i < 4 - trick_size - 1; ++i) {
                if (players_team[player_it] == players_team[player_to_move]) {
                    // play dummy card if player is in the same team
                    current_trick.set_card(player_it, no_card);
                } else {
                    vector<Move> legal_cards;
                    current_trick.get_legal_cards_for_player(player_it, legal_cards, cards[player_it]);
                    // search for the highest ranked card the player can play
                    unsigned int highest_card_rank = game_type->get_rank(legal_cards[0].get_card());
                    size_t highest_card_rank_index = 0;
                    for (size_t k = 1; k < legal_cards.size(); ++k) {
                        Card legal_card = legal_cards[k].get_card();
                        if (game_type->get_rank(legal_card) < highest_card_rank ||
                            (game_type->is_trump(legal_card) && !game_type->is_trump(legal_cards[highest_card_rank_index].get_card()))) {
                            // the second check allows a lower rank of a trump card to be of global higher rank if the highest ranked card so far is a non trump) {
                            highest_card_rank = game_type->get_rank(legal_card);
                            highest_card_rank_index = k;
                        }
                    }
                    current_trick.set_card(player_it, legal_cards[highest_card_rank_index].get_card());
                }
                player_it = next_player(player_it);
            }
            int trick_now_taken_by = current_trick.taken_by();
            if (trick_taken_by == trick_now_taken_by) {
                // trick is safe, player is free to play any valuable card
                vector<Card> possible_cards;
                for (size_t i = 0; i < legal_moves.size(); ++i) {
                    possible_cards.push_back(legal_moves[i].get_card());
                }
                int chosen_index = play_valuable_card(possible_cards);
                //cout << "returning index of a valuable card for a safe trick: " << chosen_index << endl;
                return chosen_index;
            }
            //cout << "but not safe" << endl;
        }
    }

    // ending up here means that either the current trick is empty (i.e. player to move starts a new trick) or the trick contains one or more cards already, but it is not owned by a teammate of player to move so far or it is not safe because another player can still play a higher card.
    vector<int> safe_card_indices; // set of indices of safe cards that win the trick
    set<Cards> safe_suits; // set of safe suits to play because a teammate can win the trick
    for (size_t i = 0; i < legal_moves.size(); ++i) {
        Card current_card = legal_moves[i].get_card();
        Trick current_trick1(tricks.back());
        current_trick1.set_card(player_to_move, current_card); // every player plays his highest card into this trick
        Trick current_trick2(tricks.back());
        current_trick2.set_card(player_to_move, current_card); // only opponent players play their highest card into this trick, i.e. in the end, if the trick is owned by player to move, the card is a safe card.
        // iterate over the next players, let them play their highest ranked legal card. if player_to_move wins the trick, the card played is a safe card; if a teammate wins the trick, the played card's suit is safe and otherwise, no good card can be played.
        int player_it = next_player(player_to_move);
        for (int j = 0; j < 4 - trick_size - 1; ++j) {
            vector<Move> legal_cards;
            current_trick1.get_legal_cards_for_player(player_it, legal_cards, cards[player_it]);
            // search for the highest ranked card the player can play
            unsigned int highest_card_rank = game_type->get_rank(legal_cards[0].get_card());
            size_t highest_card_rank_index = 0;
            for (size_t k = 1; k < legal_cards.size(); ++k) {
                Card legal_card = legal_cards[k].get_card();
                if (game_type->get_rank(legal_card) < highest_card_rank ||
                    (game_type->is_trump(legal_card) && !game_type->is_trump(legal_cards[highest_card_rank_index].get_card()))) {
                    // the second check allows a lower rank of a trump card to be of global higher rank if the highest ranked card so far is a non trump
                    highest_card_rank = game_type->get_rank(legal_card);
                    highest_card_rank_index = k;
                }
            }
            if (players_team[player_it] != players_team[player_to_move]) {
                current_trick2.set_card(player_it, legal_cards[highest_card_rank_index].get_card());
            } else {
                current_trick2.set_card(player_it, no_card);
            }
            current_trick1.set_card(player_it, legal_cards[highest_card_rank_index].get_card());
            player_it = next_player(player_it);
        }
        int trick1_taken_by = current_trick1.taken_by();
        int trick2_taken_by = current_trick2.taken_by();
        if (trick2_taken_by == player_to_move) {
            // player to move can play a safe card that wins the trick
            safe_card_indices.push_back(i);
        } else {
            assert(trick1_taken_by != player_to_move); // if player to move did not win trick 2, he cannot have won trick 1 where more players have played their cards.
            if (players_team[trick1_taken_by] == players_team[player_to_move]) {
                // a teammate of player to move won the trick, i.e. the played card's suit is safe to play
                safe_suits.insert(game_type->get_suit(current_card));
            }
        }
    }

    // iterate over all safe cards
    for (size_t i = 0; i < safe_card_indices.size(); ++i) {
        Card current_card = legal_moves[safe_card_indices[i]].get_card();
        if (game_type->is_trump(current_card)) // prefer to play a safe non trump card over playing a safe trump card
            continue;
        int chosen_index = safe_card_indices[i];
        //cout << "returning index of a safe non trump card:  " << chosen_index << endl;
        return chosen_index;
    }
    // if we are here, then because all safe cards are only trump cards or because there are no safe cards at all. we even prefer playing a safe suit where a teammate can win the trick over playing a safe trump card because playing a high trump card is many times a waste.
    for (set<Cards>::const_iterator it = safe_suits.begin(); it != safe_suits.end(); ++it) {
        if (*it == game_type->get_trump_suit()) // prefer playing a safe non trump suit over playing trump
            continue;
        vector<Card> possible_cards;
        vector<size_t> index_to_index(legal_moves.size());
        for (size_t i = 0; i < legal_moves.size(); ++i) {
            Card current_card = legal_moves[i].get_card();
            if (it->contains_card(current_card)) {
                possible_cards.push_back(current_card);
                index_to_index[possible_cards.size() - 1] = i;
            }
        }
        int chosen_index = play_valuable_card(possible_cards);
        chosen_index = index_to_index[chosen_index];
        //cout << "returning index of a valuable card for safe non trump suit: " << chosen_index << endl;
        return chosen_index;
    }
    // if we are here, then because all safe cards are only trump cards or because there are no safe cards at all and because the only safe suit to play is the trump suit or because there is no safe suit at all. prefer playing a safe trump card if exists over playing a low trump card.
    if (!safe_card_indices.empty()) {
        int chosen_index = safe_card_indices.front(); // legal moves are ordered in ascending rank order and thus the first element in safe_card_indices is the lowest safe trump card
        // TODO: better play valuable safe low trump card
        assert(game_type->is_trump(legal_moves[chosen_index].get_card()));
        //cout << "returning index of a safe trump card: " << chosen_index << endl;
        return chosen_index;
    }
    // if we are here, then because there is no safe card at all and no safe non trump suit.
    if (!safe_suits.empty()) {
        assert(safe_suits.size() == 1); // only the trump suit can be a safe suit
        assert(*safe_suits.begin() == game_type->get_trump_suit());
        vector<Card> possible_cards;
        vector<size_t> index_to_index(legal_moves.size());
        for (size_t i = 0; i < legal_moves.size(); ++i) {
            Card current_card = legal_moves[i].get_card();
            if (safe_suits.begin()->contains_card(current_card)) {
                possible_cards.push_back(current_card);
                index_to_index[possible_cards.size() - 1] = i;
            }
        }
        int chosen_index = play_valuable_card(possible_cards);
        chosen_index = index_to_index[chosen_index];
        //cout << "returning index of a valuable card for safe trump suit: " << chosen_index << endl;
        return chosen_index;
    }
    // if we are here, then because there is neither a safe card nor a safe suit to play.
    return -1;
}
