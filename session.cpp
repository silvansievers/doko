#include "session.h"

#include "actual_game_state.h"
#include "game_type.h"
#include "human_player.h"
#include "options.h"
#include "random_player.h"
#include "rng.h"
#include "timer.h"
#include "uct_player.h"

#include <algorithm>
#include <cassert>
#include <iostream>

using namespace std;

Session::Session(Options &options_)
    : options(options_),
      rng(options.get_random_seed()),
      first_player(0),
      vorfuehrung(false) {
    timer = new Timer();
    Cards::setup_bit_count();
    play();
    cout << "time: " << *timer << endl;
}

Session::~Session() {
    for (int i = 0; i < 4; ++i)
        delete players[i];
    delete timer;
}

void Session::play() {
    cout << "starting doppelkopf session" << endl << endl;
    options.dump();
    const vector<player_t> &players_types = options.get_players_types();
    for (size_t i = 0; i < 4; ++i) {
        switch (players_types[i]) {
            case UCT:
                players[i] = new UctPlayer(i, options);
                break;
            case HUMAN:
                players[i] = new HumanPlayer(i);
                break;
            case RANDOM:
                players[i] = new RandomPlayer(i);
                break;
        }
        players_points[i] = 0;
        if (options.use_compulsory_solo())
            played_compulsory_solo[i] = false;
        else // no compulsory solo to play is the same as saying players already played their compulsory solo
            played_compulsory_solo[i] = true;
    }
    for (int i = 0; i < options.get_number_of_games(); ++i) {
        cout << "starting game number " << i << " [" << *timer << "]" << endl;
        if (options.use_random_cards())
            shuffle_cards();
        else {
            if (!options.specify_cards_manually(cards)) {
                cout << "using random cards" << endl;
                shuffle_cards();
            }
        }
        set_cards();
        if (!vorfuehrung)
            check_vorfuehrung(options.get_number_of_games() - i);
        ActualGameState actual_game_state(options, players, first_player, cards, played_compulsory_solo, vorfuehrung);
        int new_points[4] = { 0, 0, 0, 0 };
        actual_game_state.get_score_points(new_points);
        for (int i = 0; i < 4; ++i) {
            players[i]->inform_about_game_end(new_points);
            players_points[i] += new_points[i];
        }
        if (actual_game_state.is_compulsory_solo()) {
            int solo_player = actual_game_state.get_compulsory_solo_player();
            played_compulsory_solo[solo_player] = true;
        }
        statistics();
        // update next player for all normal games and vorfuehrung compulsory solos. all "regular" compulsory solos are going to be repeated with the same dealer and player positions (although for vorfuehrung it does not really matter, because anyway the solo player starts playing)
        if (!actual_game_state.is_compulsory_solo() || vorfuehrung)
            first_player = next_player(first_player);
    }
    cout << "doppelkopf session finished" << endl;
}

void Session::shuffle_cards() {
    cards[0] = Cards();
    cards[1] = Cards();
    cards[2] = Cards();
    cards[3] = Cards();

    Card deck[48];
    for (int i = 0; i < 48; ++i) {
        deck[i] = Card(i);
    }

    random_shuffle(deck, deck + 48, rng);

    for (int i = 0; i < 12; ++i) {
        cards[0].add_card(deck[i]);
        cards[1].add_card(deck[i + 12]);
        cards[2].add_card(deck[i + 24]);
        cards[3].add_card(deck[i + 36]);
    }

    assert(cards[0].size() == 12);
    assert(cards[1].size() == 12);
    assert(cards[2].size() == 12);
    assert(cards[3].size() == 12);
}

void Session::set_cards() const {
    //if (options.use_verbose())
        //cout << "dealing cards:" << endl;
    for (int i = 0; i < 4; ++i) {
        //if (options.use_verbose())
            //cout << i << " " << cards[i] << endl;
        players[i]->set_cards(cards[i]);
    }
}

void Session::check_vorfuehrung(int number_of_remaining_games) {
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

void Session::statistics() const {
    cout << "standings:" << endl;
    int sum = 0;
    for (int i = 0; i < 4; ++i) {
        cout << "player " << i << ": " << players_points[i] << endl;
        sum += players_points[i];
    }
    cout << endl;
    assert(sum == 0);
}
