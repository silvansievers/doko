#include "human_player.h"

#include "game_type.h"

#include <cassert>
#include <iostream>
#include <string>

using namespace std;

HumanPlayer::HumanPlayer(int player_number) : Player(player_number), game_type(0) {
}

void HumanPlayer::set_cards(Cards cards_) {
    Player::set_cards(cards_);
    game_type = 0;
}

static unsigned int read_int_in_range(int min_value, int max_value) {
    while (true) {
        int value;
        cin >> value;
        if (cin.fail() || value < 0) {
            cin.clear();
            string wayne;
            getline(cin, wayne);
        } else if (value >= min_value && value <= max_value) {
            return static_cast<unsigned int>(value);
        }
        cout << "invalid option, try again:" << endl;
    }
}

size_t HumanPlayer::make_move(const vector<Move> &legal_moves) const {
    for (size_t i = 0; i < legal_moves.size(); ++i) {
        cout << i << " - ";
        legal_moves[i].print_option();
        if (i != legal_moves.size() - 1)
            cout << ", ";
    }
    cout << endl;
    return read_int_in_range(0, legal_moves.size());
}

size_t HumanPlayer::make_card_move(const vector<Move> &legal_moves) const {
    // in order to allow a more human readable input, i.e. H9 to play hearts 9, this method needs to do some extra computation as creating a Cards object with all cards and in the end iterate over legal_moves to find the index of the card chosen by the player
    Cards legal_cards;
    for (size_t i = 0; i < legal_moves.size(); ++i)
        legal_cards.add_card(legal_moves[i].get_card());
    Card card_to_play;
    while (true) { // repeated input until player choses one card of his hand
        string name;
        while (true) { // check for valid input, i.e. any card (not necessarily owned by the player, though)
            cin >> name;
            if (!is_valid_card_name(name)) {
                cout << "unknown input, try again:" << endl;
            } else {
                break;
            }
        }
        pair<Card, Card> card_pair = get_cards_for_name(name);
        if (legal_cards.contains_card(card_pair.first)) {
            card_to_play = card_pair.first;
            break;
        }
        if (legal_cards.contains_card(card_pair.second)) {
            card_to_play = card_pair.second;
            break;
        }
        cout << "you cannot play this card!" << endl;
    }
    for (size_t i = 0; i < legal_moves.size(); ++i)
        if (legal_moves[i].get_card() == card_to_play)
            return i;

    // this is the alternative to the more complicated computations above: simply ask the user for the index of the displayed card
    /*cout << "please type in the appropriate index of the above output, starting from 0 for the first option" << endl;
    int value = read_int_in_range(0, legal_moves.size());
    return value;*/

    assert(false);
    return 0;
}

size_t HumanPlayer::ask_for_move(const vector<Move> &legal_moves) {
    cout << "you are player " << id << ", these are your cards: " << endl;
    cards.show(game_type);
    // a card move
    if (legal_moves[0].is_card_move()) {
        cout << "please play a card (type in 'H9' or 'C1' for hearts 9 or clubs 10 e.g.)" << endl;
        return make_card_move(legal_moves);
    }
    // not a card move
    if (legal_moves[0].is_question_move()) {
        question_t question_type = legal_moves[0].get_question_type();
        switch (question_type) {
            case IMMEDIATE_SOLO:
                cout << "do you want to play an immediate solo (remaining players will not be asked for reservation)?" << endl;
                break;
            case HAS_RESERVATION:
                cout << "do you have a reservation?" << endl;
                break;
            case IS_SOLO:
                cout << "do you want to play a solo?" << endl;
                break;
        }
    } else if (legal_moves[0].is_game_type_move()) {
        cout << "which game type do you want to play?" << endl;
    } else if (legal_moves[0].is_announcement_move()) {
        cout << "do you want to make an announcement (if yes, which)?" << endl;
    }
    return make_move(legal_moves);
}

void HumanPlayer::inform_about_move(int player, const Move &move) {
    Player::inform_about_move(player, move);
    if (move.is_game_type_move()) {
        assert(game_type == 0); // no other game_type has been set before
        game_type = move.get_game_type();
    } else if (game_type == 0 && move.is_card_move()) {
        // if game_type is still not set when the first card is played, then nobody plays a solo/marriage, thus it is a regular game
        game_type = &regular;
    }
}
