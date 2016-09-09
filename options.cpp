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

#include "options.h"

#include <cassert>
#include <iostream>
#include <string>

using namespace std;

Options::Options(int number_of_games_, bool no_solo_, bool compulsory_solo_, const vector<player_t> &players_types_,
                 bool random_cards_, int random_seed_, bool verbose_, bool uct_verbose_, bool debug_,
                 bool uct_debug_, const vector<vector<int> > &players_options_, bool create_graph_,
                 int announcing_version_)
                 : number_of_games(number_of_games_), no_solo(no_solo_), compulsory_solo(compulsory_solo_),
                   players_types(players_types_), random_cards(random_cards_), random_seed(random_seed_),
                   verbose(verbose_), uct_verbose(uct_verbose_), debug(debug_), uct_debug(uct_debug_),
                   players_options(players_options_), create_graph(create_graph_), announcing_version(announcing_version_) {
}

bool Options::specify_cards_manually(Cards cards[4]) const {
    cout << "random cards option not set - do you want to specify a card distribution? (y)es or (n)o" << endl;
    cout << "choosing no will deal random cards" << endl;
    while (true) {
        char answer;
        cin >> answer;
        if (answer == 'n')
            return false;
        else if (answer == 'y')
            break;
        else
            cout << "invalid answer, please type in 'y' or 'n'" << endl;
    }

    cout << "do you want to manually specify a card distribution? type in 0" << endl;
    cout << "otherwise, chose a predefined distribution: type in 1 or 2" << endl;
    int value;
    while (true) {
        cin >> value;
        if (cin.fail() || value < 0 || value > 2) {
            cin.clear();
            string wayne;
            getline(cin, wayne);
            cout << "invalid option, try again:" << endl;
        } else
            break;
    }

    if (value == 0) {
        Cards used_cards;
        for (int i = 0; i < 4; ++i) { // enter cards for all players
            Cards cards_player_i;
            cout << "enter 12 cards for player " << i << endl;
            while (cards_player_i.size() < 12) {
                string name;
                while (true) { // check for valid input, i.e. any card
                    cin >> name;
                    if (!is_valid_card_name(name)) {
                        cout << "unknown input, try again:" << endl;
                    } else {
                        break;
                    }
                }
                pair<Card, Card> card_pair = get_cards_for_name(name);
                if (used_cards.contains_card(card_pair.first)) {
                    if (used_cards.contains_card(card_pair.second)) {
                        cout << "both of these cards have beeen distributed, chose another one" << endl;
                        continue;
                    } else {
                        cards_player_i.add_card(card_pair.second);
                        used_cards.add_card(card_pair.second);
                        continue;
                    }
                } else {
                    cards_player_i.add_card(card_pair.first);
                    used_cards.add_card(card_pair.first);
                    continue;
                }
            }
            cards[i] = cards_player_i;
            cout << "set player " << i << "'s cards to:" << cards_player_i << endl;
        }
    } else {
        Cards cards1, cards2, cards3, cards4;
        switch (value) {
            case 1:
                cards1 = Card(H1 | H1_ | CQ | CQ_ | HQ | DQ | DA | C1 | CK | C9 | SA | S9);
                cards2 = Card(SQ | HQ_ | CJ | SJ | HJ | D1 | DK | CA | C1_ | S9_ | HA | HK);
                cards3 = Card(SQ_ | DQ_ | SJ_ | HJ_ | DJ | D9 | CA_ | SA_ | S1 | SK | HK_ | H9);
                cards4 = Card(CJ_ | DJ_ | DA_ | D1_ | DK_ | D9_ | CK_ | C9_ | S1_ | SK_ | H9_ | HA_);
                break;
            case 2:
                cards1 = Card(SA | SA_ | S1 | S1_ | SK | SK_ | S9 | S9_ | CA | CA_ | C1 | C1_);
                cards2 = Card(CQ | CQ_ | H1 | H1_ | SQ | SQ_ | HQ | HQ_ | DQ | DQ_ | CJ | CJ_);
                cards3 = Card(SJ | SJ_ | HJ | HJ_ | DJ | DJ_ | DA | DA_ | D1 | D1_ | DK | DK_);
                cards4 = Card(D9 | D9_ | HA | HA_ | HK | HK_ | H9 | H9_ | CK | CK_ | C9 | C9_);
        }
        cards[0] = cards1;
        cards[1] = cards2;
        cards[2] = cards3;
        cards[3] = cards4;
    }
    return true;
}

void print(const string &output, bool value) {
    cout << output << (value ? "yes" : "no") << endl;
}

void Options::dump() const {
    cout << "Chosen options for the session:" << endl;
    cout << "Number of games: " << number_of_games << endl;
    print("Solo playing disabled: ", no_solo);
    print("Play compulsory solos: ", compulsory_solo);
    print("Random cards: ", random_cards);
    cout << "Random seed: " << random_seed << endl;
    cout << "Announcing version: " << announcing_version << endl;
    //print("Verbose: ", verbose);
    for (size_t i = 0; i < players_types.size(); ++i) {
        cout << "\nPlayer " << i << "'s type: ";
        switch (players_types[i]) {
            case UCT:
                cout << "UCT" << endl;
                //print("Uct verbose: ", uct_verbose);
                //print("Debug: ", debug);
                cout << "UCT version: " << players_options[i][0] << endl;
                cout << "Score points constant: " << players_options[i][1] << endl;
                print("Use team's points instead of player's points: ", players_options[i][2]);
                cout << "Playing points constant: " << players_options[i][3] << endl;
                cout << "Exploration constant: " << players_options[i][4] << endl;
                cout << "Number of rollouts: " << players_options[i][5] << endl;
                if (players_options[i][0] == 0)
                    cout << "Number of simulations: " << players_options[i][6] << endl;
                cout << "Announcement rule: ";
                switch (players_options[i][7]) {
                    case 0:
                        cout << "no" << endl;
                        break;
                    case 1:
                        cout << "yes" << endl;
                        break;
                    case 2:
                        cout << "only +" << endl;
                        break;
                    default:
                        assert(false);
                }
                print("Use wrong UCT formula: ", players_options[i][8]);
                print("Use MC simulation: ", players_options[i][9]);
                cout << "Action selection: " << players_options[i][10] << endl;
                break;
            case HUMAN:
                cout << "Human" << endl;
                break;
            case RANDOM:
                cout << "Random" << endl;
                break;
        }
    }
    cout << endl;
}
