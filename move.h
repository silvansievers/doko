#ifndef MOVE_H
#define MOVE_H

#include "cards.h"

#include <cassert>
#include <iostream>
#include <fstream>
#include <vector>

enum question_t {
    IMMEDIATE_SOLO,
    HAS_RESERVATION,
    IS_SOLO
};

class GameType;

class Move {
private:
    enum move_t {
        QUESTION,
        GAME_TYPE,
        ANNOUNCEMENT,
        CARD
    };
    move_t type;
    question_t question_type;
    bool answer_re; // being used to store the answer for a question or if announcing player belongs to re team or not
    const GameType *game_type;
    announcement_t announcement;
    Card card;

    void print_type(std::ostream &out) const;
public:
    Move() {} // introduced for UctPlayer
    Move(question_t question_type, bool answer);
    explicit Move(const GameType *game_type);
    Move(announcement_t announcement, bool re_team);
    explicit Move(Card card);
    bool is_question_move() const {
        return type == QUESTION;
    }
    question_t get_question_type() const {
        assert(is_question_move());
        return question_type;
    }
    bool get_answer() const {
        return answer_re;
    }
    bool is_game_type_move() const {
        return type == GAME_TYPE;
    }
    const GameType *get_game_type() const {
        assert(is_game_type_move());
        return game_type;
    }
    bool is_announcement_move() const {
        return type == ANNOUNCEMENT;
    }
    announcement_t get_announcement() const {
        assert(is_announcement_move());
        return announcement;
    }
    bool get_re_team() const {
        return answer_re;
    }
    bool is_card_move() const {
        return type == CARD;
    }
    Card get_card() const {
        assert(is_card_move());
        return card;
    }

    bool operator==(const Move &rhs) const { // TODO: this is only for debugging (assertion in uct_player.cpp)
        if (type != rhs.type)
            return false;
        if (type == QUESTION)
            return (question_type == rhs.question_type && answer_re == rhs.answer_re);
        else if (type == GAME_TYPE)
            return game_type == rhs.game_type; // NOTE: comparing the pointers actually is enough because they all point to the same global const objects defined in game_type.h
        else if (type == ANNOUNCEMENT)
            return announcement == rhs.announcement;
        else if (type == CARD)
            return card == rhs.card;
        else
            return false;
    }
    void print_option(std::ostream &out = std::cout) const; // public for human_player
    friend std::ostream &operator<<(std::ostream &out, const Move &move);
    friend std::ostream &operator<<(std::ostream &out, const std::vector<Move> &moves);
};

#endif
