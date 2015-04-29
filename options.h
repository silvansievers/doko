#ifndef OPTIONS_H
#define OPTIONS_H

#include "cards.h"

#include <vector>

enum player_t {
    UCT,
    HUMAN,
    RANDOM
};

class Options {
private:
    int number_of_games;
    bool no_solo;
    bool compulsory_solo;
    std::vector<player_t> players_types;
    bool random_cards;
    int random_seed;
    bool verbose;
    bool uct_verbose;
    bool debug;
    bool uct_debug;
    std::vector<std::vector<int> > players_options;
    bool create_graph;
    int announcing_version;
public:
    Options(int number_of_games, bool no_solo, bool compulsory_solo, const std::vector<player_t> &players_types,
            bool random, int random_seed, bool verbose, bool uct_verbose, bool debug, bool uct_debug,
            const std::vector<std::vector<int> > &players_options, bool create_graph, int announcing_version);
    int get_number_of_games() const {
        return number_of_games;
    }
    bool solo_disabled() const {
        return no_solo;
    }
    bool use_compulsory_solo() const {
        return compulsory_solo;
    }
    const std::vector<player_t> &get_players_types() const {
        return players_types;
    }
    bool use_random_cards() const {
        return random_cards;
    }
    int get_random_seed() const {
        return random_seed;
    }
    bool use_verbose() const {
        return verbose;
    }
    bool use_uct_verbose() const {
        return uct_verbose;
    }
    bool use_debug() const {
        return debug;
    }
    bool use_uct_debug() const {
        return uct_debug;
    }
    int get_uct_version(int player) const {
        return players_options[player][0];
    }
    int get_score_points_constant(int player) const {
        return players_options[player][1];
    }
    bool use_team_points(int player) const {
        return (players_options[player][2] == 1);
    }
    int get_playing_points_constant(int player) const {
        return players_options[player][3];
    }
    int get_exploration_constant(int player) const {
        return players_options[player][4];
    }
    int get_number_of_rollouts(int player) const {
        return players_options[player][5];
    }
    int get_number_of_simulations(int player) const {
        return players_options[player][6];
    }
    int get_announcement_option(int player) const {
        return players_options[player][7];
    }
    bool use_wrong_uct_formula(int player) const {
        return players_options[player][8];
    }
    int get_simulation_option(int player) const {
        return players_options[player][9];
    }
    int get_action_selection_version(int player) const {
        return players_options[player][10];
    }
    bool use_create_graph() const {
        return create_graph;
    }
    int get_announcing_version() const {
        return announcing_version;
    }
    bool specify_cards_manually(Cards cards[4]) const; // return true iff user specifies cards manually, false iff he decides to use a random distribution
    void dump() const;
};

#endif
