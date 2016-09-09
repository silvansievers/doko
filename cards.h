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

/*
  The idea of implementing cards using a single long long value stems from
  the Skat player by Sebastian Kupferschmid, released under the GNU GENERAL
  PUBLIC LICENSE Version 2, described in "Entwicklung eines Double-Dummy Skat
  Solvers mit einer Anwendung für verdeckte Skatspiele" (Sebastian Kupferschmid,
  University of Freiburg, 2003).
*/

#ifndef CARDS_H
#define CARDS_H

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

// NOTE: this file also serves as a general files for definitions, because all other files need to include cards.h
enum announcement_t {
    NONE,
    REKON,
    N90,
    N60,
    N30,
    SCHWARZ
};

const int NUM_ANNOUNCEMENTS = 6;

class GameType;

class Card {
private:
    unsigned long long value;
    static const int card_to_index[53];
    static const char *const card_names[48];
    static const int card_values[48];
public:
    Card();
    Card(int number);

    // non class function get_cards_for_name needs access to card_names
    static const char *const *get_card_names() {
        return card_names;
    }

    int get_index() const { // returns the index of the card if value is a valid card value and -1 otherwise
        return card_to_index[value % 53];
    }
    const char* get_name() const {
        if (value == 0)
            return "";
        return card_names[get_index()];
    }
    int get_value() const {
        assert(value != 0);
        return card_values[get_index()];
    }

    // NOTE: this operator violates Card's invariant of being a single card! it is only there for the ease of constructing Cards from Card, where Card is the bitwise or of many single cards. the problem is that neither Cards operator|(const Card &rhs) nor operator Cards() is possible, because Cards is not known when Card is being declared/defined.
    Card operator|(const Card &rhs) const {
        Card cards(*this);
        cards.value |= rhs.value;
        return cards;
    }
    Card &operator=(const Card &rhs) { // this operator is probably not necessary because the compiler could infer it
        value = rhs.value;
        return *this;
    }
    bool operator==(const Card &rhs) const {
        return value == rhs.value;
    }
    bool operator!=(const Card &rhs) const {
        return value != rhs.value;
    }
    friend std::ostream &operator<<(std::ostream &out, Card card);
    friend class Cards;
};

class Cards {
private:
    unsigned long long value;
    static char bits_in_16bits[65536];
    static const int m1  = 0x5555; //binary: 0101...
    static const int m2  = 0x3333; //binary: 00110011..
    static const int m4  = 0x0f0f; //binary:  4 zeros,  4 ones ...
    static const int m8  = 0x00ff; //binary:  8 zeros,  8 ones ...

    void print(std::ostream &out, const GameType *game_type) const;
public:
    Cards();
    Cards(const Card &card);

    static void setup_bit_count();

    void add_card(const Card &card) {
        value |= card.value;
    }
    void add_cards(const Cards &cards) {
        value |= cards.value;
    }
    void remove_card(const Card &card);
    void remove_cards(const Cards &cards);
    bool contains_card(const Card &card) const {
        return (value & card.value) != 0;
    }
    Cards get_intersection(const Cards &cards) const;
    int size() const;
    bool empty() const {
        return size() == 0;
    }
    void get_single_cards(std::vector<Card> &cards) const;

    Cards &operator=(const Cards &rhs) { // this operator is probably not necessary because the compiler could infer it
        value = rhs.value;
        return *this;
    }
    bool operator==(const Cards &rhs) const { // added for BeliefGameState==
        return value == rhs.value;
    }
    bool operator<(const Cards &rhs) const { // needed for set<Cards> in BeliefGameState::get_best_move_index
        return value < rhs.value;
    }

    void show(const GameType *game_type) const;
    friend std::ostream &operator<<(std::ostream &out, Cards card);
};

// NOTE: this cannot be in class Card, because it should return objects of type Card, and it should not be in class Cards either, because it does not really belong there.
bool is_valid_card_name(const std::string &name);
std::pair<Card, Card> get_cards_for_name(const std::string &name);

// needed in so many different places that it is global now
int next_player(int player);

const Card no_card;

const Card H9(0);
const Card H9_(1);
const Card HK(2);
const Card HK_(3);
const Card HA(4);
const Card HA_(5);

const Card S9(6);
const Card S9_(7);
const Card SK(8);
const Card SK_(9);
const Card S1(10);
const Card S1_(11);
const Card SA(12);
const Card SA_(13);

const Card C9(14);
const Card C9_(15);
const Card CK(16);
const Card CK_(17);
const Card C1(18);
const Card C1_(19);
const Card CA(20);
const Card CA_(21);

const Card D9(22);
const Card D9_(23);
const Card DK(24);
const Card DK_(25);
const Card D1(26);
const Card D1_(27);
const Card DA(28);
const Card DA_(29);

const Card DJ(30);
const Card DJ_(31);
const Card HJ(32);
const Card HJ_(33);
const Card SJ(34);
const Card SJ_(35);
const Card CJ(36);
const Card CJ_(37);

const Card DQ(38);
const Card DQ_(39);
const Card HQ(40);
const Card HQ_(41);
const Card SQ(42);
const Card SQ_(43);
const Card CQ(44);
const Card CQ_(45);

const Card H1(46);
const Card H1_(47);

const Cards jacks(DJ | HJ | SJ | CJ | DJ_ | HJ_ | SJ_ | CJ_);
const Cards queens(DQ | HQ | SQ | CQ | DQ_ | HQ_ | SQ_ | CQ_);
const Cards hearts10s(H1 | H1_);

const Cards diamonds(D9 | DK | D1 | DA | D9_ | DK_ | D1_ | DA_);
const Cards hearts(H9 | HK | HA | H9_ | HK_ | HA_);
const Cards spades(S9 | SK | S1 | SA | S9_ | SK_ | S1_ | SA_);
const Cards clubs(C9 | CK | C1 | CA | C9_ | CK_ | C1_ | CA_);

#endif
