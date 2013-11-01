#include "cards.h"

#include "game_type.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>

using namespace std;

Card::Card() : value(0) {
}

Card::Card(int number) : value(1ULL << number) {
}

const int Card::card_to_index[53]= {
    -1, 0, 1, 17, 2, 47, 18, 14, 3, 34,
    -1, 6, 19, 24, 15, 12, 4, 10, 35, 37,
    -1, 31, 7, 39, 20, 42, 25, -1, 16, 46,
    13, 33, 5, 23, 11, 9, 36, 30, 38, 41,
    -1, 45, 32, 22, 8, 29, 40, 44, 21, 28,
    43, 27, 26
};

const char *const Card::card_names[48] = {
    "H9", "H9", "HK", "HK", "HA", "HA",
    "S9", "S9", "SK", "SK", "S1", "S1", "SA", "SA",
    "C9", "C9", "CK", "CK", "C1", "C1", "CA", "CA",
    "D9", "D9", "DK", "DK", "D1", "D1", "DA", "DA",
    "DJ", "DJ", "HJ", "HJ", "SJ", "SJ", "CJ", "CJ",
    "DQ", "DQ", "HQ", "HQ", "SQ", "SQ", "CQ", "CQ",
    "H1", "H1"
};

const int Card::card_values[48] = {
    0, 0, 4, 4, 11, 11,
    0, 0, 4, 4, 10, 10, 11, 11,
    0, 0, 4, 4, 10, 10, 11, 11,
    0, 0, 4, 4, 10, 10, 11, 11,
    2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3,
    10, 10
};

ostream &operator<<(ostream &out, Card card) {
    out << "[" << card.get_name() << "]";
    return out;
}

// Compares two given Cards according to their rank, depending on the GameType
struct CardComparator {
    const GameType *game_type;
    CardComparator(const GameType *game_type_) : game_type(game_type_) {}
    bool operator()(Card lhs, Card rhs) const {
        bool lhs_is_trump = game_type->get_trump_suit().contains_card(lhs);
        bool rhs_is_trump = game_type->get_trump_suit().contains_card(rhs);
        if (lhs_is_trump != rhs_is_trump) { // one card is trump, the other is not
            return lhs_is_trump;
        } else {
            if (lhs_is_trump) { // both cards are trump
                return game_type->get_trump_rank(lhs) <= game_type->get_trump_rank(rhs);
            } else { // both cards are non trump
                const vector<Cards> &non_trump_suits = game_type->get_non_trump_suits();
                for (size_t i = non_trump_suits.size() - 1; i >= 0; --i) {
                    bool lhs_is_of_current_suit = non_trump_suits[i].contains_card(lhs);
                    bool rhs_is_of_current_suit = non_trump_suits[i].contains_card(rhs);
                    if (lhs_is_of_current_suit && rhs_is_of_current_suit) // both cards are of the current non trump suit
                        return game_type->get_non_trump_rank(lhs) <= game_type->get_non_trump_rank(rhs);
                    if (lhs_is_of_current_suit) // only lhs is of the current non trump suit, thus ordering it before rhs
                        return true;
                    if (rhs_is_of_current_suit) // only rhs is of the current non trump suit, thus ordering lhs behind rhs
                        return false;
                    if (i == 0) { // avoid letting get i negative (which would cause an overflow, because its of type size_t)
                        assert(false); // in the last iteration, one of the three tests above MUST succeed! thus we never reach this
                        break;
                    }
                }
            }
            assert(false);
            return true;
        }
    }
};

Cards::Cards() : value(0) {
}

Cards::Cards(const Card &card) : value(card.value) {
}

void Cards::print(ostream &out, const GameType *game_type) const {
    vector<Card> single_cards;
    get_single_cards(single_cards);
    sort(single_cards.begin(), single_cards.end(), CardComparator(game_type));
    out << "[";
    for (size_t i = 0; i < single_cards.size(); ++i) {
        out << single_cards[i].get_name();
        if (i != single_cards.size() - 1)
            out << ", ";
    }
    out << "]";
}

char Cards::bits_in_16bits[65536] = {""};

void Cards::setup_bit_count() {
    for (int i = 0; i < 65536; ++i) {
        // from wikipedia: Hamming_weight
        int x = (i & m1 ) + ((i >>  1) & m1 ); //put count of each  2 bits into those  2 bits
        x = (x & m2 ) + ((x >>  2) & m2 ); //put count of each  4 bits into those  4 bits
        x = (x & m4 ) + ((x >>  4) & m4 ); //put count of each  8 bits into those  8 bits
        x = (x & m8 ) + ((x >>  8) & m8 ); //put count of each 16 bits into those 16 bits
        bits_in_16bits[i] = x;
    }
}

void Cards::remove_card(const Card &card) {
    assert(contains_card(card));
    value ^= card.value;
}

void Cards::remove_cards(const Cards &cards) {
    vector<Card> single_cards;
    cards.get_single_cards(single_cards);
    for (size_t i = 0; i < single_cards.size(); ++i) {
        Card card = single_cards[i];
        if (contains_card(single_cards[i])) {
            remove_card(card);
        }
    }
    // TODO: why does this not work?
    /*value ^= cards.value;
    vector<Card> single_cards;
    cards.get_single_cards(single_cards);
    for (size_t i = 0; i < single_cards.size(); ++i)
        assert(!contains_card(single_cards[i]));*/
}

Cards Cards::get_intersection(const Cards &cards) const {
    Cards cards_intersection;
    cards_intersection.value = (value & cards.value);
    return cards_intersection;
}

int Cards::size() const {
    int result = bits_in_16bits[value & 0xffffu]
    + bits_in_16bits[(value >> 16) & 0xffffu]
    + bits_in_16bits[(value >> 32) & 0xffffu];
    return result;
}

void Cards::get_single_cards(vector<Card> &cards) const {
    cards.reserve(size());
    for (int i = 0; i < 48; ++i) {
        Card card(i);
        if (contains_card(card))
            cards.push_back(card);
    }
}

void Cards::show(const GameType *game_type) const {
    if (game_type == 0) // cannot use a default parameter game_type = &regular because game_type uses Card/Cards classes
        game_type = &regular;
    print(cout, game_type);
    cout << endl;
}

ostream &operator<<(ostream &out, Cards cards) {
    cards.print(out, &regular);
    return out;
}

static map<string, pair<Card, Card> > name_to_card;

bool is_valid_card_name(const string &name) {
    if (name_to_card.empty()) {
        for (size_t i = 0; i < 48; i += 2) {
            assert(string(Card::get_card_names()[i]) == string(Card::get_card_names()[i + 1]));
            name_to_card[Card::get_card_names()[i]] = make_pair(Card(i), Card(i + 1));
        }
    }
    return name_to_card.count(name);
}

pair<Card, Card> get_cards_for_name(const string &name) {
    assert(is_valid_card_name(name));
    return name_to_card[name];
}

int next_player(int player) {
    return (player + 1) % 4;
}
