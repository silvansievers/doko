#include "player.h"

#include <cassert>
#include <iostream>

using namespace std;

Player::Player(int player_number) : id(player_number) {
}

void Player::set_cards(Cards cards_) {
    cards = cards_;
}

void Player::invalid_move() const {
    cout << "player " << id << " somehow managed to return an invalid index" << endl;
    assert(false);
}

void Player::inform_about_move(int player, const Move &move) {
    if (move.is_card_move() && id == player) {
        cards.remove_card(move.get_card());
    }
}
