#include "move.h"

#include "game_type.h"

#include <cassert>
#include <iostream>

using namespace std;

Move::Move(question_t question_type_, bool answer) : type(QUESTION), question_type(question_type_), answer_re(answer) {
}

Move::Move(const GameType *game_type_) : type(GAME_TYPE), game_type(game_type_) {
}

Move::Move(announcement_t announcement_, bool re_team) : type(ANNOUNCEMENT), answer_re(re_team), announcement(announcement_)  {
}

Move::Move(Card card_) : type(CARD), card(card_) {
}

void Move::print_type(ostream &out) const {
    out << "move type: ";
    switch (type) {
        case QUESTION:
            switch (question_type) {
                case IMMEDIATE_SOLO:
                    out << "immediate solo: ";
                    break;
                case HAS_RESERVATION:
                    out << "has reservation: ";
                    break;
                case IS_SOLO:
                    out << "is solo: ";
                    break;
            }
            break;
        case GAME_TYPE:
            out << "game type: ";
            break;
        case ANNOUNCEMENT:
            out << "announcement: ";
            break;
        case CARD:
            out << "card: ";
            break;
    }
}

void Move::print_option(ostream &out) const {
    switch (type) {
        case QUESTION:
            out << (answer_re ? "yes" : "no");
            break;
        case GAME_TYPE:
            out << *game_type;
            break;
        case ANNOUNCEMENT:
            switch (announcement) {
                case NONE:
                    out << "no announcement";
                    break;
                case REKON:
                    out << "re/kontra";
                    break;
                case N90:
                    out << "no 90";
                    break;
                case N60:
                    out << "no 60";
                    break;
                case N30:
                    out << "no 30";
                    break;
                case SCHWARZ:
                    out << "black";
                    break;
            }
            break;
        case CARD:
            assert(false);
            break;
    }
}

ostream &operator<<(ostream &out, const Move &move) {
    move.print_type(out);
    if (move.is_card_move())
        out << move.get_card();
    else
        move.print_option(out);
    return out;
}

ostream &operator<<(ostream &out, const vector<Move> &moves) {
    moves[0].print_type(out);
    out << endl;
    out << "move options: ";
    for (size_t i = 0; i < moves.size(); ++i) {
        if (moves[i].is_card_move())
            out << moves[i].get_card();
        else
            moves[i].print_option(out);
        if (i != moves.size() - 1)
            out << ", ";
    }
    return out;
}
