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

#include "game_type.h"

using namespace std;

GameType::GameType(int type_) {
    type = static_cast<game_t>(type_);
    if (type <= CLUBS_SOLO) { // regular, marriage or color solo => hearts 10s, queens and jacks are trump and there are 3 non trump suits
        trump_suit.add_cards(hearts10s);
        card_to_trump_rank[H1.get_index()] = 1;
        card_to_trump_rank[H1_.get_index()] = 1;
        trump_suit.add_cards(queens);
        card_to_trump_rank[CQ.get_index()] = 2;
        card_to_trump_rank[CQ_.get_index()] = 2;
        card_to_trump_rank[SQ.get_index()] = 3;
        card_to_trump_rank[SQ_.get_index()] = 3;
        card_to_trump_rank[HQ.get_index()] = 4;
        card_to_trump_rank[HQ_.get_index()] = 4;
        card_to_trump_rank[DQ.get_index()] = 5;
        card_to_trump_rank[DQ_.get_index()] = 5;
        trump_suit.add_cards(jacks);
        card_to_trump_rank[CJ.get_index()] = 6;
        card_to_trump_rank[CJ_.get_index()] = 6;
        card_to_trump_rank[SJ.get_index()] = 7;
        card_to_trump_rank[SJ_.get_index()] = 7;
        card_to_trump_rank[HJ.get_index()] = 8;
        card_to_trump_rank[HJ_.get_index()] = 8;
        card_to_trump_rank[DJ.get_index()] = 9;
        card_to_trump_rank[DJ_.get_index()] = 9;
        non_trump_suits.resize(3);
        if (type <= DIAMONDS_SOLO) { // regular, marriage or diamonds solo => all with the same trump/non trump suits
            trump_suit.add_cards(diamonds);
            card_to_trump_rank[DA.get_index()] = 10;
            card_to_trump_rank[DA_.get_index()] = 10;
            card_to_trump_rank[D1.get_index()] = 11;
            card_to_trump_rank[D1_.get_index()] = 11;
            card_to_trump_rank[DK.get_index()] = 12;
            card_to_trump_rank[DK_.get_index()] = 12;
            card_to_trump_rank[D9.get_index()] = 13;
            card_to_trump_rank[D9_.get_index()] = 13;
            non_trump_suits[0] = hearts;
            non_trump_suits[1] = spades;
            non_trump_suits[2] = clubs;
        } else if (type == HEARTS_SOLO) {
            trump_suit.add_cards(hearts);
            card_to_trump_rank[HA.get_index()] = 10;
            card_to_trump_rank[HA_.get_index()] = 10;
            card_to_trump_rank[HK.get_index()] = 11;
            card_to_trump_rank[HK_.get_index()] = 11;
            card_to_trump_rank[H9.get_index()] = 12;
            card_to_trump_rank[H9_.get_index()] = 12;
            non_trump_suits[0] = diamonds;
            non_trump_suits[1] = spades;
            non_trump_suits[2] = clubs;
        } else if (type == SPADES_SOLO) {
            trump_suit.add_cards(spades);
            card_to_trump_rank[SA.get_index()] = 10;
            card_to_trump_rank[SA_.get_index()] = 10;
            card_to_trump_rank[S1.get_index()] = 11;
            card_to_trump_rank[S1_.get_index()] = 11;
            card_to_trump_rank[SK.get_index()] = 12;
            card_to_trump_rank[SK_.get_index()] = 12;
            card_to_trump_rank[S9.get_index()] = 13;
            card_to_trump_rank[S9_.get_index()] = 13;
            non_trump_suits[0] = diamonds;
            non_trump_suits[1] = hearts;
            non_trump_suits[2] = clubs;
        } else if (type == CLUBS_SOLO) {
            trump_suit.add_cards(clubs);
            card_to_trump_rank[CA.get_index()] = 10;
            card_to_trump_rank[CA_.get_index()] = 10;
            card_to_trump_rank[C1.get_index()] = 11;
            card_to_trump_rank[C1_.get_index()] = 11;
            card_to_trump_rank[CK.get_index()] = 12;
            card_to_trump_rank[CK_.get_index()] = 12;
            card_to_trump_rank[C9.get_index()] = 13;
            card_to_trump_rank[C9_.get_index()] = 13;
            non_trump_suits[0] = diamonds;
            non_trump_suits[1] = hearts;
            non_trump_suits[2] = spades;
        }
    } else { // jacks, queens or aces solo => all colors (except jacks/queens) are non trump suits
        non_trump_suits.resize(4);
        non_trump_suits[0] = diamonds;
        non_trump_suits[1] = hearts;
        non_trump_suits[1].add_cards(hearts10s); // include hearts 10s in the hearts suit
        non_trump_suits[2] = spades;
        non_trump_suits[3] = clubs;
        if (type == JACKS_SOLO || type == ACES_SOLO) { // jacks or aces solo => add queens to non trump suits
            // include all queens to their respective suit
            non_trump_suits[0].add_card(DQ);
            non_trump_suits[0].add_card(DQ_);
            non_trump_suits[1].add_card(HQ);
            non_trump_suits[1].add_card(HQ_);
            non_trump_suits[2].add_card(SQ);
            non_trump_suits[2].add_card(SQ_);
            non_trump_suits[3].add_card(CQ);
            non_trump_suits[3].add_card(CQ_);
            if (type == JACKS_SOLO) {
                trump_suit.add_cards(jacks);
                card_to_trump_rank[CJ.get_index()] = 1;
                card_to_trump_rank[CJ_.get_index()] = 1;
                card_to_trump_rank[SJ.get_index()] = 2;
                card_to_trump_rank[SJ_.get_index()] = 2;
                card_to_trump_rank[HJ.get_index()] = 3;
                card_to_trump_rank[HJ_.get_index()] = 3;
                card_to_trump_rank[DJ.get_index()] = 4;
                card_to_trump_rank[DJ_.get_index()] = 4;
            }
        }
        if (type == QUEENS_SOLO || type == ACES_SOLO) { // queens or aces solo => add jacks to non trump suits
            // include all jacks to their respective suit
            non_trump_suits[0].add_card(DJ);
            non_trump_suits[0].add_card(DJ_);
            non_trump_suits[1].add_card(HJ);
            non_trump_suits[1].add_card(HJ_);
            non_trump_suits[2].add_card(SJ);
            non_trump_suits[2].add_card(SJ_);
            non_trump_suits[3].add_card(CJ);
            non_trump_suits[3].add_card(CJ_);
            if (type == QUEENS_SOLO) {
                trump_suit.add_cards(queens);
                card_to_trump_rank[CQ.get_index()] = 1;
                card_to_trump_rank[CQ_.get_index()] = 1;
                card_to_trump_rank[SQ.get_index()] = 2;
                card_to_trump_rank[SQ_.get_index()] = 2;
                card_to_trump_rank[HQ.get_index()] = 3;
                card_to_trump_rank[HQ_.get_index()] = 3;
                card_to_trump_rank[DQ.get_index()] = 4;
                card_to_trump_rank[DQ_.get_index()] = 4;
            }
        }
    }

    // initialize card_to_suit
    for (int i = 0; i < 48; i++) {
        Card card(i);
        if (trump_suit.contains_card(card)) {
            card_to_suit[card.get_index()] = trump_suit;
        }
        else {
            for (size_t j = 0; j < non_trump_suits.size(); ++j) {
                if (non_trump_suits[j].contains_card(card)) {
                    card_to_suit[card.get_index()] = non_trump_suits[j];
                }
            }
        }
    }
}

/* maps a non trump card to its rank in its suit.
this ranking is the same for all type of games: ace, 10, king, queen, jack, 9 for all colors, where ace has highest rank (i.e. rank 1). if queens or jacks or 10s are in fact non trump cards, then they are included in the trump suit and also in the card_to_trump_rank array.
this array is being accessed with the help of get_index
*/
static const unsigned int card_to_non_trump_rank[48] = {
    6, 6, 3, 3, 1, 1,
    6, 6, 3, 3, 2, 2, 1, 1,
    6, 6, 3, 3, 2, 2, 1, 1,
    6, 6, 3, 3, 2, 2, 1, 1,
    5, 5, 5, 5, 5, 5, 5, 5,
    4, 4, 4, 4, 4, 4, 4, 4,
    2, 2
};

unsigned int GameType::get_non_trump_rank(Card card) const {
    return card_to_non_trump_rank[card.get_index()];
}

ostream &operator<<(ostream &out, const GameType &game_type) {
    switch(game_type.type) {
        case GameType::REGULAR:
            out << "regular";
            break;
        case GameType::MARRIAGE:
            out << "marriage";
            break;
        case GameType::DIAMONDS_SOLO:
            out << "diamonds solo";
            break;
        case GameType::HEARTS_SOLO:
            out << "hearts solo";
            break;
        case GameType::SPADES_SOLO:
            out << "spades solo";
            break;
        case GameType::CLUBS_SOLO:
            out << "clubs solo";
            break;
        case GameType::JACKS_SOLO:
            out << "jacks solo";
            break;
        case GameType::QUEENS_SOLO:
            out << "queens solo";
            break;
        case GameType::ACES_SOLO:
            out << "aces solo";
            break;
    }
    return out;
}

const GameType regular(0);
const GameType marriage(1);
const GameType diamonds_solo(2);
const GameType hearts_solo(3);
const GameType spades_solo(4);
const GameType clubs_solo(5);
const GameType jacks_solo(6);
const GameType queens_solo(7);
const GameType aces_solo(8);
