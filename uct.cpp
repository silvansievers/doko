#include "uct.h"

#include "belief_game_state.h"
#include "card_assignment.h"
#include "move.h"
#include "options.h"

#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>

using namespace std;

const double EPSILON = 0.0000001;

Node::Node(const int id_, int player_to_move_) : id(id_), player_to_move(player_to_move_), parent(0), num_visits(0)/**,
    player_to_move2(player_to_move_), parent2(0), num_visits2(0)*/ {
    for (int i = 0; i < 4; ++i) {
        accumulated_reward[i] = 0.0;
        ///accumulated_reward2[i] = 0.0;
    }
}

Node::~Node() {
    for (size_t i = 0; i < successors.size(); ++i) {
        delete successors[i];
    }
}

void Node::dump() const {
    cout << "node id: " << id << endl;
    cout << "player to move: " << player_to_move << endl;
    ///cout << "player to move2: " << player_to_move2 << endl;
    cout << "number of successors: " << successors.size() << endl;
    ///cout << "number of successors2: " << successors2.size() << endl;
    cout << "number of visits: " << num_visits << endl;
    ///cout << "number of visits2: " << num_visits2 << endl;
    cout << "accumulated players' rewards: ";
    for (int i = 0; i < 4; ++i)
        cout << i << ": " << accumulated_reward[i] << " ";
    cout << endl;
    /**cout << "accumulated players' rewards2: ";
    for (int i = 0; i < 4; ++i)
        cout << i << ": " << accumulated_reward2[i] << " ";
    cout << endl;*/
}

/**static void check_node_consistency(Node *current_node) {
    assert(current_node->player_to_move == current_node->player_to_move2);
    assert(current_node->parent == current_node->parent2);
    assert(current_node->successors.size() <= current_node->successors2.size());
    //assert(current_node->moves.size() == current_node->moves2.size());
    for (size_t i = 0; i < current_node->successors.size(); ++i) {
        bool same_successor_found = false;
        for (size_t j = 0; j < current_node->successors2.size(); ++j) {
            if (current_node->successors[i] == current_node->successors2[j]) {
                assert(current_node->moves[i] == current_node->moves2[j]);
                same_successor_found = true;
                break;
            }
        }
        assert(same_successor_found);
    }
    assert(current_node->num_visits == current_node->num_visits2);
    for (int i = 0; i < 4; ++i)
        assert(current_node->accumulated_reward[i] == current_node->accumulated_reward2[i]);
}*/

Uct::Uct(const Options &options_, const BeliefGameState &state, Cards players_cards, int move_number_)
    : options(options_), uct_player(state.get_player_to_move()), rng(2011), move_number(move_number_),
    nodes_counter(0) {
    if (options.use_uct_verbose())
        cout << endl << "uct instance " << uct_player << " beginning" << endl;
    assert(players_cards.size() >= 1);

    if (options.get_uct_version(uct_player) == 1) {
        root = new Node(0, uct_player);
        CardAssignment card_assignment(options, state, players_cards);
        for (int i = 0; i < options.get_number_of_rollouts(uct_player); ++i) {
            BeliefGameState *start_state = new BeliefGameState(state);
            start_state->set_uct_output(false);
            card_assignment.assign_cards_to_players(*start_state);
            if (i == 0 && options.use_uct_verbose()) {
                vector<Move> legal_moves;
                start_state->get_legal_moves(legal_moves);
                cout << legal_moves << endl;
            }
            rollout(start_state, i);
            delete start_state;
        }
        if (average_rewards.empty()) {
            average_rewards.resize(root->successors.size(), 0.0);
            move_indices_count.resize(root->successors.size(), 0);
        }
        for (size_t j = 0; j < root->successors.size(); ++j) {
            average_rewards[j] += root->successors[j]->accumulated_reward[uct_player] / root->successors[j]->num_visits;
        }
        unsigned int best_move = calculate_best_move_index(root, options.get_number_of_rollouts(uct_player));
        ++move_indices_count[best_move];
    } else {
        CardAssignment card_assignment(options, state, players_cards);
        for (int i = 0; i < options.get_number_of_simulations(uct_player); ++i) {
            if (options.use_uct_debug() && move_number == 11)
                cout << "starting simulation number " << i << endl;
            root = new Node(0, uct_player);
            nodes_counter = 0;
            BeliefGameState *one_state = new BeliefGameState(state);
            one_state->set_uct_output(false);
            card_assignment.assign_cards_to_players(*one_state);
            if (i == 0 && options.use_uct_verbose()) {
                vector<Move> legal_moves;
                one_state->get_legal_moves(legal_moves);
                cout << legal_moves << endl;
            }
            belief_game_states.push_back(one_state);
            for (int j = 0; j < options.get_number_of_rollouts(uct_player); ++j) {
                BeliefGameState *start_state = new BeliefGameState(*one_state);
                rollout(start_state, j);
                delete start_state;
            }
            ///check_node_consistency(root);
            if (average_rewards.empty()) {
                average_rewards.resize(root->successors.size(), 0.0);
                move_indices_count.resize(root->successors.size(), 0);
            }
            for (size_t j = 0; j < root->successors.size(); ++j) {
                if (root->successors[j] == 0) // if number of rollouts is set to be smaller than the number of possible moves for the player then stop the loop as soon as encountering a non expanded node
                    break;
                average_rewards[j] += root->successors[j]->accumulated_reward[uct_player] / root->successors[j]->num_visits;
            }
            ///unsigned int best_move2[1] = { 0 };
            unsigned int best_move = calculate_best_move_index(root, options.get_number_of_rollouts(uct_player)/**, best_move2*/);
            //assert(best_move == best_move2[0]);
            ++move_indices_count[best_move];
            if (options.use_create_graph())
                dot(i);
            delete root;
        }
    }
}

Uct::~Uct() {
    if (options.get_uct_version(uct_player) == 1)
        delete root;
    else {
        for (size_t i = 0; i < belief_game_states.size(); ++i)
            delete belief_game_states[i];
    }
}

unsigned int Uct::calculate_best_move_index(Node *node,  int number_of_rollout, /*unsigned int *max_index2, */bool with_exploration_term,
                                            const BeliefGameState *state, unsigned int *move_index) const {
    /** for debugging purposes
    vector<pair<size_t, size_t> > correct_version;
    vector<pair<size_t, size_t> > wrong_version;
    for (size_t i = 0; i < node->successors2.size(); ++i) {
        //cout << "index i: " << i << endl;
        //node->successors2[i]->dump();
        if (node->successors2[i] == 0)
            continue;
        for (size_t j = 0; j < node->successors.size(); ++j) {
            //cout << "index j: " << j << endl;
            //node->successors[j]->dump();
            if (node->successors2[i] == node->successors[j]) {
                correct_version.push_back(make_pair(i, j));
                //if (move_number == 11 && node->id == 372)
                    //cout << "found matching successors at index i " << i << " and j " << j << endl;
                break;
            }
        }
    }
    //if (move_number == 11 && node->id == 372)
        //cout << endl;
    for (size_t j = 0; j < node->successors.size(); ++j) {
        //cout << "index j: " << j << endl;
        //node->successors[j]->dump();
        //if (node->successors2[i] == 0)
            //continue;
        for (size_t i = 0; i < node->successors2.size(); ++i) {
            //cout << "index i: " << i << endl;
            //node->successors2[i]->dump();
            if (node->successors2[i] == node->successors[j]) {
                wrong_version.push_back(make_pair(i, j));
                //if (move_number == 11 && node->id == 372)
                    //cout << "found matching successors at index i " << i << " and j " << j << endl;
                break;
            }
        }
    }
    assert(correct_version.size() == wrong_version.size());
    for (size_t i = 0; i < correct_version.size(); ++i) {
        const pair<size_t, size_t> &correct_pair = correct_version[i];
        bool found_matching_pair = false;
        for (size_t j = 0; j < wrong_version.size(); ++j) {
            const pair<size_t, size_t> &wrong_pair = wrong_version[j];
            if (correct_pair.first == wrong_pair.first && correct_pair.second == wrong_pair.second) {
                found_matching_pair = true;
                break;
            }
        }
        assert(found_matching_pair);
        //assert(correct_version[i].first == wrong_version[i].first);
        //assert(correct_version[i].second == wrong_version[i].second);
    }
    int max_index_ = -1;
    double best_reward_so_far2 = 0;
    bool only_negative_rewards2 = true;
    // NOTE: "Bug" was found: the order in which the successors occur can have an influence in the case that two or more successors are tied for being the "best" one
    for (size_t j = 0; j < node->successors.size(); ++j) {
        for (size_t i = 0; i < node->successors2.size(); ++i) {
            if (node->successors2[i] == node->successors[j]) {
                assert(j != -1);
                int current_move_index = -1; // for uct version 1, need to store the index of vector legal moves and write it to move_index in the end. max_index is still needed for accessing the right successor
                //if (options.get_uct_version(uct_player) == 1) {
                    assert(node->successors[j] != 0);
                    if (with_exploration_term) { // if this method is called by get_best_move, then node == root and at root, all card assignments  yield the same successors because the uct player himself is being asked to play, thus only for the other cases, need to check if the current successor is actually consistent to the current card assignment. if not, do not consider it for computations.
                        vector<Move> legal_moves;
                        state->get_legal_moves(legal_moves);
                        assert(legal_moves.size() > 0);
                        bool move_contained = false;
                        for (size_t k = 0; k < legal_moves.size(); ++k) {
                            if (node->moves[j] == legal_moves[k]) {
                                // NOTE: explanations about this extra check see "NOTE" below, at rollout(), line 253
                                BeliefGameState state_copy(*state);
                                state_copy.set_move(state_copy.get_player_to_move(), legal_moves[k]);
                                if (state_copy.get_player_to_move() == node->successors[j]->player_to_move
                                    || node->successors[j]->player_to_move == -2) { // TODO: the second check is needed only if uct version 0 also uses moves which are not ordered  (introcude at the second place below, too)
                                    move_contained = true;
                                    current_move_index = k;
                                    break;
                                }
                            }
                        }
                        if (!move_contained)
                            continue;
                        assert(legal_moves[current_move_index] == node->moves[j]);
                        //assert(legal_moves[current_move_index] == node->moves2[j]);
                    }
                //} else {
                    // if number of rollouts is set to be smaller than the number of possible moves for the player then stop the loop as soon as encountering a non expanded node (because nodes are expanded in the order of increasing indices)
                    if (node->successors2[i] == 0)
                        break;
                //}

                bool updated_index_version0 = false;
                int num_visits = node->successors2[i]->num_visits2;
                double current_reward = node->successors2[i]->accumulated_reward2[node->player_to_move2] / num_visits;
                if (options.get_announcement_option(uct_player) == 2 && !with_exploration_term
                    && root->moves2[0].is_announcement_move() && current_reward > 0) {
                    only_negative_rewards = false;
                }
                double reward_copy = current_reward;
                double exploration_term = static_cast<double>(options.get_exploration_constant(uct_player)) * sqrt(log(options.use_wrong_uct_formula() ? number_of_rollout : node->num_visits2) / num_visits);
                if (options.use_uct_verbose() && (options.get_uct_version(uct_player) == 1 && !with_exploration_term)) {
                    cout << "index " << i << ":";
                    cout << " num visits: " << num_visits;
                    cout << " average reward: " << current_reward;
                    cout << " exploration term: " << exploration_term << endl;
                    cout << "index " << i << " yields a result of " << current_reward << endl;
                }
                if (with_exploration_term) {
                    current_reward += static_cast<double>(options.get_exploration_constant(uct_player)) * sqrt(log(options.use_wrong_uct_formula() ? number_of_rollout : node->num_visits2) / num_visits);
                    reward_copy += exploration_term;
                    //assert(current_reward == reward_copy);
                }
                //if (move_number == 11 && node->id == 372) {
                    cout << "best reward so far: " << best_reward_so_far << endl;
                    cout << "current reward: " << current_reward << endl;
                    cout << "index pair: i " << i << ", " << j << endl;
                }//
                if (max_index == -1 || (current_reward - best_reward_so_far) > EPSILON) {
                    if ((current_reward - best_reward_so_far) > EPSILON && (reward_copy - best_reward_so_far) <= EPSILON) {
                        cout << setprecision(25);
                        cout << "ERROR" << endl;
                        cout << "best reward so far: " << best_reward_so_far << endl;
                        cout << "current reward: " << current_reward << endl;
                        cout << "reward copy: " << reward_copy << endl;
                        exit(2);
                    }
                    //if (move_number == 11 && node->id == 372) {
                        cout << "best reward so far: " << best_reward_so_far << endl;
                        cout << "updating to: " << current_reward << endl;
                        cout << "index pair: i " << i << ", " << j << endl;
                    }//
                    best_reward_so_far = current_reward;
                    //if (options.get_uct_version(uct_player) == 1 && with_exploration_term) {
                        assert(current_move_index >= 0);
                        move_index[0] = static_cast<unsigned int>(current_move_index);
                    }//
                    max_index = i;
                    updated_index_version0 = true;
                }

                bool updated_index_version1 = false;
                int num_visits2 = node->successors[j]->num_visits;
                double current_reward2 = node->successors[j]->accumulated_reward[node->player_to_move] / num_visits2;
                if (options.get_announcement_option(uct_player) == 2 && !with_exploration_term
                    && root->moves[0].is_announcement_move() && current_reward2 > 0) {
                    only_negative_rewards2 = false;
                }
                double reward_copy2 = current_reward2;
                double exploration_term2 = static_cast<double>(options.get_exploration_constant(uct_player)) * sqrt(log(options.use_wrong_uct_formula() ? number_of_rollout : node->num_visits2) / num_visits2);
                if (options.use_uct_verbose() && (options.get_uct_version(uct_player) == 1 && !with_exploration_term)) {
                    cout << "index " << j << ":";
                    cout << " num visits: " << num_visits2;
                    cout << " average reward: " << current_reward2;
                    cout << " exploration term: " << exploration_term2 << endl;
                    cout << "index " << j << " yields a result of " << current_reward2 << endl;
                }
                if (with_exploration_term) {
                    current_reward2 += static_cast<double>(options.get_exploration_constant(uct_player)) * sqrt(log(options.use_wrong_uct_formula() ? number_of_rollout : node->num_visits2) / num_visits2);
                    reward_copy2 += exploration_term2;
                    //assert(current_reward == reward_copy);
                }
                //if (move_number == 11 && node->id == 372) {
                    cout << "best reward so far: " << best_reward_so_far2 << endl;
                    cout << "current reward: " << current_reward2 << endl;
                    cout << "index pair: i " << i << ", " << j << endl;
                }//
                if (max_index_ == -1 || (current_reward2 - best_reward_so_far2) > EPSILON) {
                    if ((current_reward2 - best_reward_so_far2) > EPSILON && (reward_copy2 - best_reward_so_far2) <= EPSILON) {
                        cout << setprecision(25);
                        cout << "ERROR" << endl;
                        cout << "best reward so far: " << best_reward_so_far2 << endl;
                        cout << "current reward: " << current_reward2 << endl;
                        cout << "reward copy: " << reward_copy2 << endl;
                        exit(2);
                    }
                    //if (move_number == 11 && node->id == 372) {
                        cout << "best reward so far: " << best_reward_so_far2 << endl;
                        cout << "updating to: " << current_reward2 << endl;
                        cout << "index pair: i " << i << ", " << j << endl;
                    }//
                    best_reward_so_far2 = current_reward2;
                    if (//options.get_uct_version(uct_player) == 1 && //with_exploration_term) {
                        assert(current_move_index >= 0);
                        move_index[0] = static_cast<unsigned int>(current_move_index);
                    }
                    max_index_ = j;
                    updated_index_version1 = true;
                }
                if (updated_index_version0 != updated_index_version1) {
                    cout << "move 0: " << node->moves[i] << endl;
                    cout << "move 1: " << node->moves2[j] << endl;
                    cout << "current reward 0: " << current_reward << endl;
                    cout << "best reward so far 1: " << best_reward_so_far2 << endl;
                    cout << "current reward 1: " << current_reward2 << endl;
                    cout << "best reward so far 1: " << best_reward_so_far2 << endl;
                }
                assert(updated_index_version0 == updated_index_version1);
                break;
            }
        }
    }
    assert(max_index >= 0);
    assert(max_index_ >= 0);
    if (options.get_announcement_option(uct_player) == 2 && !with_exploration_term
        && root->moves2[0].is_announcement_move() && only_negative_rewards) {
        if (options.use_uct_verbose())
            cout << "all successors yield a negative reward, forbid announcing" << endl;
        return 0;
    }
    max_index2[0] = max_index_;
    //if (move_number == 11 && node->id == 372) {
    //    cout << "returning index pair: i " << max_index << ", " << max_index2[0] << endl;
    //    //exit(0);
    //}*/

    int max_index = -1;
    double best_reward_so_far = 0;
    bool only_negative_rewards = true;
    for (size_t i = 0; i < node->successors.size(); ++i) {
        int current_move_index = -1; // for uct version 1, need to store the index of vector legal moves and write it to move_index in the end. max_index is still needed for accessing the right successor
        if (options.get_uct_version(uct_player) == 1) {
            assert(node->successors[i] != 0);
            if (with_exploration_term) { // if this method is called by get_best_move, then node == root and at root, all card assignments  yield the same successors because the uct player himself is being asked to play, thus only for the other cases, need to check if the current successor is actually consistent to the current card assignment. if not, do not consider it for computations.
                vector<Move> legal_moves;
                state->get_legal_moves(legal_moves);
                assert(legal_moves.size() > 0);
                bool move_contained = false;
                for (size_t j = 0; j < legal_moves.size(); ++j) {
                    if (node->moves[i] == legal_moves[j]) {
                        // NOTE: explanations about this extra check see "NOTE" below, at rollout(), line 512
                        BeliefGameState state_copy(*state);
                        state_copy.set_move(state_copy.get_player_to_move(), legal_moves[j]);
                        if (state_copy.get_player_to_move() == node->successors[i]->player_to_move) {
                            // NOTE: || node->successors[i]->player_to_move == -2 needs to be added as an additional check if also using version 0 to use this style of only adding successors to a node which are actually going to be visited and not all of them as it is done right now. No explanation found why version 1 does not require this extra check...
                            move_contained = true;
                            current_move_index = j;
                            break;
                        }
                    }
                }
                if (!move_contained)
                    continue;
                assert(legal_moves[current_move_index] == node->moves[i]);
            }
        } else {
            // if number of rollouts is set to be smaller than the number of possible moves for the player then stop the loop as soon as encountering a non expanded node (because nodes are expanded in the order of increasing indices)
            if (node->successors[i] == 0)
                break;
        }

        int num_visits = node->successors[i]->num_visits;
        double current_reward = node->successors[i]->accumulated_reward[node->player_to_move] / num_visits;
        if (options.get_announcement_option(uct_player) == 2 && !with_exploration_term
            && root->moves[0].is_announcement_move() && current_reward > 0) {
            only_negative_rewards = false;
        }
        double reward_copy = current_reward;
        double exploration_term = static_cast<double>(options.get_exploration_constant(uct_player)) * sqrt(log(options.use_wrong_uct_formula(uct_player) ? number_of_rollout : node->num_visits) / num_visits);
        if (options.use_uct_verbose() && (options.get_uct_version(uct_player) == 1 && !with_exploration_term)) {
            cout << "index " << i << ":";
            cout << " num visits: " << num_visits;
            cout << " average reward: " << current_reward;
            cout << " exploration term: " << exploration_term << endl;
            cout << "index " << i << " yields a result of " << current_reward << endl;
        }
        if (with_exploration_term) {
            current_reward += static_cast<double>(options.get_exploration_constant(uct_player)) * sqrt(log(options.use_wrong_uct_formula(uct_player) ? number_of_rollout : node->num_visits) / num_visits);
            reward_copy += exploration_term;
            //assert(current_reward == reward_copy);
        }
        if (max_index == -1 || (current_reward - best_reward_so_far) > EPSILON) {
            if ((current_reward - best_reward_so_far) > EPSILON && (reward_copy - best_reward_so_far) <= EPSILON) {
                cout << setprecision(25);
                cout << "ERROR" << endl;
                cout << "best reward so far: " << best_reward_so_far << endl;
                cout << "current reward: " << current_reward << endl;
                cout << "reward copy: " << reward_copy << endl;
                exit(2);
            }
            best_reward_so_far = current_reward;
            if (options.get_uct_version(uct_player) == 1 && with_exploration_term) {
                assert(current_move_index >= 0);
                move_index[0] = static_cast<unsigned int>(current_move_index);
            }
            max_index = i;
        }
    }
    assert(max_index >= 0);
    if (options.get_announcement_option(uct_player) == 2 && !with_exploration_term
        && root->moves[0].is_announcement_move() && only_negative_rewards) {
        if (options.use_uct_verbose())
            cout << "all successors yield a negative reward, forbid announcing" << endl;
        return 0;
    }
    return static_cast<unsigned int>(max_index);
}

void Uct::propagate_values(Node *current_node, BeliefGameState *current_state) {
    // current_node is the last visited node (which is in most of the cases not coinciding with current_state!)
    assert(current_state->game_finished());
    int score_points[4] = { 0, 0, 0, 0 };
    int players_points[4] = { 0, 0, 0, 0 };
    int team_points[2] = { 0, 0 };
    double uct_rewards[4];
    current_state->get_score_points(score_points, players_points, team_points);
    for (int i = 0; i < 4; ++i) {
        uct_rewards[i] = options.get_score_points_constant(uct_player) * score_points[i] + static_cast<double>((options.use_team_points(uct_player) ? team_points[current_state->get_players_team(i)] : players_points[i])) / options.get_playing_points_constant(uct_player);
        if (options.use_uct_debug()) {
            cout << "player " << i << "'s score points: " << score_points[i] << endl;
            cout << "player " << i << "'s uct rewards: " << uct_rewards[i] << endl;
        }
    }
    if (options.use_uct_debug())
        cout << endl;
    while (current_node != 0) { // go back to root (whose parent is 0)
        ++(current_node->num_visits);
        ///++(current_node->num_visits2);
        for (int i = 0; i < 4; ++i) {
            current_node->accumulated_reward[i] += uct_rewards[i];
            ///current_node->accumulated_reward2[i] += uct_rewards[i];
        }
        ///assert(current_node->parent == current_node->parent2);
        current_node = current_node->parent;
    }
}

void Uct::rollout(BeliefGameState *current_state, int number_of_rollout) {
    assert(number_of_rollout == root->num_visits);
    if (options.use_uct_debug() && move_number == 11)
        cout << "\nstarting one rollout" << endl;
    Node *current_node = root;
    ///BeliefGameState state2(*current_state);
    ///assert(*current_state == state2);
    bool added_new_node = false; // this will be set to true as soon as the first node needs to be inserted. from then on, a MC simulation will be carried out either with adding further nodes to the tree or not, depending on the chosen options
    while (true) {
        if (options.use_uct_debug()/* && !added_new_node*/) {
            // !added_new_node to avoid to print all newly created nodes as soon as a node was added (if the corresponding option to add all nodes is set)
            if (current_node != root)
                cout << endl;
            cout << "current node:" << endl;
            current_node->dump();
        }
        ///check_node_consistency(current_node);
        ///assert(*current_state == state2);

        // test if a terminal node or a terminal state (which is not the same if nodes are not added to tree as soon as one was added but the simulation is carried on) was reached
        if (current_node->player_to_move == -2 || current_state->game_finished()) {
            ///assert(current_node->player_to_move2 == -2);
            if (options.use_uct_debug())
                cout << "found an end of game leaf node or a terminal state was reached (when simulating and not adding nodes)" << endl;
            propagate_values(current_node, current_state);
            return;
        }

        // check if there are unvisited successors or if a node was already added and thus just choose an arbitrary move
        vector<Move> legal_moves;
        current_state->get_legal_moves(legal_moves);
        if (options.use_uct_debug())
            cout << "legal moves for current_state: " << legal_moves << endl;
        int chosen_move = -1;
        if (!added_new_node) { // a leaf node was not reached yet, thus check if there are (consistent in the case of uct version 1) successors of the current_node which have not been visited yet and choose one
            assert(current_node->player_to_move == current_state->get_player_to_move());
            ///assert(current_node->player_to_move2 == current_state->get_player_to_move());
            vector<size_t> not_contained_moves_indices; // stores the indices of all moves from legal_moves which are not already a successor of the current node
            ///vector<size_t> not_contained_moves_indices2;
            if (options.get_uct_version(uct_player) == 1) {
                // iterate over legal moves and check if they already exists as a successor in the tree, i.e. find all moves which are not in the tree yet if therer exist some. contrary to the case of uct version 0, do not add any other moves as uninitialized successors because if the algorithm reaches the same node in another rollout again, it will have a different card assignment
                for (size_t i = 0; i < legal_moves.size(); ++i) {
                    bool move_contained = false;
                    for (size_t j = 0; j < current_node->moves.size(); ++j) {
                        if (legal_moves[i] == current_node->moves[j]) {
                            // NOTE: in some rare cases, it could happen that when a player announced black at a node and this node was reached again later with a different card assignment, then a different player was the player to play next, because depending on the teams, the team mate of the announcing player was not allowed to do anymore announcings. To fix this, the following check on equal player to move in both the current node and the current state has been introduced.
                            BeliefGameState state_copy(*current_state);
                            state_copy.set_move(state_copy.get_player_to_move(), legal_moves[i]);
                            if (state_copy.get_player_to_move() == current_node->successors[j]->player_to_move) {
                                move_contained = true;
                                break;
                            }
                        }
                    }
                    if (!move_contained)
                        not_contained_moves_indices.push_back(i);
                }
            } else {
                /**if (current_node->successors2.empty()) { // node has not been expanded: insert ALL successors, because if at any time, the algorithm will encounter this same node again and find an unvisited leaf, it will still have the same card assignment (because after each simulation, a new tree is constructed)
                    //if (options.use_uct_debug())
                        //cout << "expanding node" << endl;
                    for (size_t i = 0; i < legal_moves.size(); ++i) {
                        current_node->moves2.push_back(legal_moves[i]);
                        current_node->successors2.push_back(0);
                    }
                }
                assert(legal_moves.size() == current_node->successors2.size());
                // iterate over all successors to see if there are still unvisited ones
                for (size_t i = 0; i < current_node->successors2.size(); ++i) {
                    assert(legal_moves[i] == current_node->moves2[i]);
                    if (current_node->successors2[i] == 0) {
                        not_contained_moves_indices2.push_back(i);
                    }
                }*/
                if (current_node->successors.empty()) { // node has not been expanded: insert ALL successors, because if at any time, the algorithm will encounter this same node again and find an unvisited leaf, it will still have the same card assignment (because after each simulation, a new tree is constructed)
                    //if (options.use_uct_debug())
                    //cout << "expanding node" << endl;
                    for (size_t i = 0; i < legal_moves.size(); ++i) {
                        current_node->moves.push_back(legal_moves[i]);
                        current_node->successors.push_back(0);
                    }
                }
                assert(legal_moves.size() == current_node->successors.size());
                // iterate over all successors to see if there are still unvisited ones
                for (size_t i = 0; i < current_node->successors.size(); ++i) {
                    assert(legal_moves[i] == current_node->moves[i]);
                    if (current_node->successors[i] == 0) {
                        not_contained_moves_indices.push_back(i);
                    }
                }
            }
            ///if (!not_contained_moves_indices2.empty()) {
            if (!not_contained_moves_indices.empty()) {
                /**assert(!not_contained_moves_indices2.empty());
                assert(not_contained_moves_indices.size() == not_contained_moves_indices2.size());
                for (size_t i = 0; i < not_contained_moves_indices.size(); ++i) {
                    assert(not_contained_moves_indices[i] == not_contained_moves_indices2[i]);
                }*/
                if (legal_moves[0].is_card_move()) {
                    if (options.get_action_selection_version(uct_player) == 0 || options.get_action_selection_version(uct_player) == 2) {
                        chosen_move = not_contained_moves_indices[0];
                    } else if (options.get_action_selection_version(uct_player) == 1 || options.get_action_selection_version(uct_player) == 3) {
                        chosen_move = not_contained_moves_indices[rng.next(not_contained_moves_indices.size())];
                    } else if (options.get_action_selection_version(uct_player) == 4) {
                        vector<Move> legal_cards;
                        for (size_t i = 0; i < not_contained_moves_indices.size(); ++i) {
                            legal_cards.push_back(legal_moves[not_contained_moves_indices[i]]);
                        }
                        int legal_cards_index = current_state->get_best_move_index(legal_cards);
                        if (legal_cards_index != -1)
                            chosen_move = not_contained_moves_indices[legal_cards_index];
                        if (chosen_move == -1) // no safe card was found
                            chosen_move = not_contained_moves_indices[rng.next(not_contained_moves_indices.size())];
                    }
                } else {
                    // take the first not contained move and add the resulting new node
                    chosen_move = not_contained_moves_indices[0];
                    ///assert(chosen_move == not_contained_moves_indices2[0]);
                }
            }
        } else { // already added a node to the tree and thus only need to choose a move. depending on the chosen options, another node will be added or not (simulation only)
            if (legal_moves[0].is_card_move()) {
                if (options.get_action_selection_version(uct_player) >= 2) {
                    chosen_move = current_state->get_best_move_index(legal_moves);
                    if (chosen_move == -1) // no safe card was found
                        chosen_move = rng.next(legal_moves.size());
                    assert(chosen_move >= 0 && chosen_move < int(legal_moves.size()));
                } else
                    chosen_move = rng.next(legal_moves.size());
            }
            else // TODO: improve the simulation. for now: no announcing, no solo play
                chosen_move = 0;
            if (options.get_simulation_option(uct_player) == 0) {
                if (options.get_uct_version(uct_player) == 0) {
                    /**if (current_node->successors2.empty()) { // node has not been expanded: insert ALL successors, because if at any time, the algorithm will encounter this same node again and find an unvisited leaf, it will still have the same card assignment (because after each simulation, a new tree is constructed)
                        //if (options.use_uct_debug())
                            //cout << "expanding node" << endl;
                        for (size_t i = 0; i < legal_moves.size(); ++i) {
                            current_node->moves2.push_back(legal_moves[i]);
                            current_node->successors2.push_back(0);
                        }
                    } */
                    // TODO: duplicated code
                    if (current_node->successors.empty()) { // node has not been expanded: insert ALL successors, because if at any time, the algorithm will encounter this same node again and find an unvisited leaf, it will still have the same card assignment (because after each simulation, a new tree is constructed)
                        //if (options.use_uct_debug())
                            //cout << "expanding node" << endl;
                        for (size_t i = 0; i < legal_moves.size(); ++i) {
                            current_node->moves.push_back(legal_moves[i]);
                            current_node->successors.push_back(0);
                        }
                    }
                }
            }
        }
        ///check_node_consistency(current_node);
        ///assert(*current_state == state2);
        if (chosen_move != -1) { // already chose a node because there were either some unvisited successors left and no node was added so far or because a leaf node was reached and from then on it suffices to choose an arbitrary move (applying the uct formula would not work because no uct rewards are known for the successors of the current node)
            assert(chosen_move >= 0);
            if (options.use_uct_debug()) {
                if (!added_new_node)
                    cout << "found unvisited successor at index " << chosen_move << endl;
                else
                    cout << "choosing index " << chosen_move << endl;
            }
            ///if (!added_new_node)
                ///current_state->set_uct_output(true);
            current_state->set_move(current_state->get_player_to_move(), legal_moves[chosen_move]);
            ///state2.set_move(state2.get_player_to_move(), legal_moves[chosen_move]);
            ///current_state->set_uct_output(false);
            ///assert(*current_state == state2);
            if (!added_new_node || options.get_simulation_option(uct_player) == 0) { // no node was added yet or the chosen option requires to add all nodes encountered during a rollout
                ++nodes_counter;
                Node *next_node = new Node(nodes_counter, current_state->get_player_to_move());
                next_node->parent = current_node;
                ///next_node->parent2 = current_node;
                if (options.get_uct_version(uct_player) == 1) {
                    current_node->moves.push_back(legal_moves[chosen_move]);
                    current_node->successors.push_back(next_node);
                } else {
                    ///assert(current_node->moves2[chosen_move] == legal_moves[chosen_move]);
                    ///current_node->successors2[chosen_move] = next_node;
                    assert(current_node->moves[chosen_move] == legal_moves[chosen_move]);
                    current_node->successors[chosen_move] = next_node;
                }
                current_node = next_node;
                if (current_state->game_finished()) {
                    current_node->player_to_move = -2;
                    ///current_node->player_to_move2 = -2;
                }
                added_new_node = true;
            }
        } else { // all successors of the current node have been visited at least once and thus need to follow the one with the highest value according the uct formula
            //if (options.use_uct_debug())
                //cout << legal_moves << endl;
            unsigned int move_index[1] = { 0 };
            ///unsigned int max_index2[1] = { 0 };
            unsigned int max_index = calculate_best_move_index(current_node, number_of_rollout, /**max_index2, */
                                                               true, current_state, move_index);
            // update current_state and current_node according to the chosen move
            if (options.use_uct_debug())
                cout << "applying uct formula, choosing index " << max_index << endl;
            if (options.get_uct_version(uct_player) == 1) {
                if (options.use_uct_debug())
                    cout << "move index is " << move_index[0] << endl;
                current_state->set_move(current_state->get_player_to_move(), legal_moves[move_index[0]]);
            } else {
                /**if (!added_new_node)
                    current_state->set_uct_output(true);
                if (options.use_uct_debug() && move_number == 11) {
                    cout << "max index is: " << max_index << endl;
                    cout << "move index is: " << move_index[0] << endl;
                    cout << "max index2 is: " << max_index2[0] << endl;
                }*/
                current_state->set_move(current_state->get_player_to_move(), legal_moves[max_index]);
                ///current_state->set_move(current_state->get_player_to_move(), legal_moves[move_index[0]]);
                ///state2.set_move(state2.get_player_to_move(), legal_moves[max_index]);
                ///current_state->set_uct_output(false);
                ///assert(*current_state == state2);
            }
            ///assert(current_node->successors[max_index] == current_node->successors2[max_index2[0]]);
            current_node = current_node->successors[max_index];
        }
    }
    // should never end up here
    assert(false);
}

void Uct::dot_rec(Node *node, int &counter, ofstream &myfile) const {
    int current_counter = counter;
    if (!node->successors.empty())
        myfile << current_counter << "[label=\"player " << node->player_to_move << "\"];\n";
    for (size_t i = 0; i < node->successors.size(); ++i) {
        if (node->successors[i] != 0) {
            myfile << ++counter << "[label=\"player " << node->successors[i]->player_to_move << "\"];\n";
            myfile << current_counter << " -> " << counter << "[label=\"";
            if (node->moves[i].is_card_move())
                myfile << node->moves[i].get_card();
            else
                node->moves[i].print_option(myfile);
            myfile << "\"];\n";
            dot_rec(node->successors[i], counter, myfile);
        }
    }
}

void Uct::dot(int number_of_simulation) const {
    stringstream sstm;
    sstm << "graph/" << uct_player << "_tree_" << move_number;
    if (options.get_uct_version(uct_player) == 0)
        sstm << "_(" << number_of_simulation << ")";
    sstm << ".dot";
    const char *file_name = sstm.str().c_str();
    int counter = 0;
    ofstream myfile;
    myfile.open(file_name);
    myfile << "digraph uct_tree {\n";
    dot_rec(root, counter, myfile);
    myfile << "}";
    myfile.close();
}

void Uct::_dump(Node *node) const {
    if (node == 0) { // root == 0
        return;
    }
    cout << "node->player_to_move = " << node->player_to_move << endl;
    cout << "accumulated rewards: ";
    for (int i = 0; i < 4; ++i) {
        cout << i << ": " << node->accumulated_reward[i];
        if (i != 3)
            cout << ", ";
    }
    cout << endl;
    if (node->successors.empty()) {
        cout << "leaf node!" << endl;
    } else {
        for (size_t i = 0; i < node->successors.size(); ++i) {
            if (node->successors[i] == 0)
                cout << "no child for index " << i << endl;
            else {
                cout << "recursive call for child with index " << i << endl;
                _dump(node->successors[i]);
                cout << "back from recursive call (for successors[" << i << "]) to node with player_to_move = " << node->player_to_move << endl;
            }
        }
    }
}

void Uct::dump() const {
    cout << "Search tree:" << endl;
    _dump(root);
    cout << "end of search tree" << endl;
}

unsigned int Uct::get_best_move() const {
    if (options.get_uct_version(uct_player) == 1) {
        if (options.use_create_graph())
            dot();
        return calculate_best_move_index(root, options.get_number_of_rollouts(uct_player));
    } else {
        int max_index = -1;
        double best_avg_reward;
        int most_often_chosen_index = -1;
        int highest_count_so_far;
        bool only_negative_rewards = true;
        for (size_t i = 0; i < average_rewards.size(); ++i) {
            double current_avg_reward = average_rewards[i] / options.get_number_of_simulations(uct_player);
            if (current_avg_reward > 0)
                only_negative_rewards = false;
            if (options.use_uct_verbose())
                cout << "index " << i << " yields an average reward of " << current_avg_reward << endl;
            if (max_index == -1 || current_avg_reward > best_avg_reward) {
                best_avg_reward = current_avg_reward;
                max_index = i;
            }
            if (options.use_uct_verbose())
                cout << "index " << i << " was chosen " << move_indices_count[i] << " times" << endl;
            if (most_often_chosen_index == -1 || move_indices_count[i] > highest_count_so_far) {
                highest_count_so_far = move_indices_count[i];
                most_often_chosen_index = i;
            }
        }
        assert(max_index >= 0);
        if (options.use_uct_verbose()) {
            cout << "max index according to summed average calculation: " << max_index << endl;
            cout << "most often chosen index: " << most_often_chosen_index << endl;
            if (max_index != most_often_chosen_index)
                cout << "MAX INDEX != MOST OFTEN CHOSEN INDEX" << endl;
        }
        if (options.get_announcement_option(uct_player) == 2 && only_negative_rewards) {
            vector<Move> legal_moves;
            belief_game_states[0]->get_legal_moves(legal_moves);
            if (legal_moves[0].is_announcement_move()) {
                if (options.use_uct_verbose())
                    cout << "all successors yield a negative reward, forbid announcing" << endl;
                return 0;
            }
        }
        return static_cast<unsigned int>(max_index);
    }
}
