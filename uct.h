#ifndef UCT_H
#define UCT_H

#include "cards.h"
#include "move.h"
#include "rng.h"

#include <fstream>
#include <vector>

/**
This class implements the UCT-algorithm for Doppelkopf. It is used by uct-players to compute the best next move. There are two different version being described in the following.

Version 0:
This version uses "simulations" and "rollouts", i.e. a simulation consists of several rollouts. When the instance is created, a card assignment is computed and fixed for each simulation. All rollouts performed during one simulation then use the same card assignment to build a game tree. Thus when expanding a node, all successors are added to it (without expanding them) and thus successors are always in the order of the legal moves of the node. This is important when it comes to the part where a MC simulation is used or not (depending on the options). When reaching a leaf node, the algorithm continues to select random moves or selects a move based on a heuristic. Depending on chosen options, all nodes enounctered starting from a leaf node are added or only the first one. In both cases, when a terminal state is reached, the game values are computed, transfered into uct rewards (depending on several options, see below) and propagated back to the root. During back-propagation, all nodes visited on the path from the root to the last added node (i.e. the node corresponding to the terminal state or the node where the MC simulation was started) get updated (i.e. uct rewards get added, number of visits counter gets increased). The resulting average rewards of the root-node for each possible successor are summed up after each simulation, and if get_best_move() is called, the move with the highest (normalized by the number of simulations which would not be necessary, but which better reflects the actual reward value for each successor) average reward is returned. For comparison reasons, there is a counter for each possible move counting how many simulations would have chosen it if only this simulation was taken into account. Some tests have shown that the summed average approach is better than the "most often chosen move" approach.

Version 1:
This version is solely based on rollouts: for each rollout, a new card assignment is being used. As a consequence, nodes in the game tree from previous rollouts may become inconsistent with the current card assignment, because obviously at a player's node where he has to play  a card, the possible successors will differ from rollout to rollout with different card assignments for this player. Thus this version needs to deal nodes as information sets rather then a single state. Luckily, this can be simplified back to "node equals state" approach from version 0 for one rollout with a fixed card assignment by just ignoring all successors of a node which are not consistent with the current game world. Still there is a difference: when expanding a node, one can only add the successor that is also immediately chosen and not all of them, because the next time the algorithm encounters the same node again, the other successors that would have been added could be inconsistent again. Consequently, successors are NOT ordered like the legal moves for the corresponding state and thus when encountering a node, the algorithm first need to match the existing successors to the current legal moves. For the rest of the algorithm, this version is similar to version 0: a rollout either adds exactly one new node to the tree and then performs a simulation or it adds all nodes encountered during the simulation, depending on the chosen options. Then the game values are computed and propagated back along the visited path. There is a pitfall that needs to be taken care off: resulting from different card assignments, it can happen that in one rollout, applying a move to a state results in a different state in the means of a different player has to move next than in another rollout (e.g. when a player announces black and his teammate is already known, then this team mate cannot do any further announcements, thus this player is "jumped" in the next player is asked for an announcement. In the next rollout, this teammate may be a different player depending on the card assignment). When get_best_move() is called, the move (successor at the root node) with the best average reward is returned (no extra calculations needed).

Implementation details common for both versions:
The result of a game gets transfered into "uct rewards" by multiplying the score points of a player by a constant (set via program options) and then adding up either the player's or the player's team points made during the game, divided by another constant (also set via options). Also the exploration constant for the uct formula can be configured via options, as can be number of simulations and rollouts.
As average rewards may be floating point numbers, the class uses double to store values. Some imprecisions have been encountered by doing the same calculations in different ways, i.e. once some intermediate results are stored, once not, this may result in "different" numbers (starting with maybe the 10th position after decimal point). Thus when comparing doubles, a difference smaller than some epsilon (0000001) is allowed and still the numbers are considered being the sames, making both methods of calculations equal (better reproducability).
*/

class BeliefGameState;
class Options;

struct Node {
    const int id;
    int player_to_move;
    Node *parent;
    std::vector<Node *> successors;
    std::vector<Move> moves;
    int num_visits;
    double accumulated_reward[4];
    /**int player_to_move2;
    Node *parent2;
    std::vector<Node *> successors2;
    std::vector<Move> moves2;
    int num_visits2;
    double accumulated_reward2[4];*/
    Node(const int id, int player_to_move);
    ~Node();
    void dump() const;
};

/**
The Uct class is not implemented for computations during the last trick, i.e. it does not work if there are less than 4 cards remaining in the game. Anyway this functionality is not needed because when players have only one card left, the only legal move will always be a card play move and they only have one option, thus no need to do any computations.
*/

class Uct {
private:
    const Options &options;
    const int uct_player; // the player owning this Uct instance
    Node *root;
    mutable RandomNumberGenerator rng;
    std::vector<BeliefGameState *> belief_game_states; // TODO: just for debugging purposes to compare legal moves!
    std::vector<double> average_rewards;
    std::vector<int> move_indices_count; // only for comparison reasons, not actually used
    int move_number; // for creating the dot files in a numbered way
    int nodes_counter;

    unsigned int calculate_best_move_index(Node *node, int number_of_rollout, /*unsigned int *max_index2,*/
                                           bool with_exploratin_term = false, const BeliefGameState *state = 0,
                                           unsigned int *move_index = 0) const;
    void propagate_values(Node *current_node, BeliefGameState *current_state);
    void rollout(BeliefGameState *current_state, int number_of_rollout);
    void dot_rec(Node *node, int &counter, std::ofstream &myfile) const;
    void _dump(Node *node) const;
public:
    Uct(const Options &options, const BeliefGameState &state, Cards players_cards, int move_number);
    ~Uct();
    void dot(int number_of_simulation = -1) const; // creates a file called tree.dot which can be converted into a graph diagram using the dot tool
    void dump() const;
    unsigned int get_best_move() const;
    const std::vector<BeliefGameState *> &get_belief_game_states() const { // see above (belief_game_states)
        return belief_game_states;
    }
};

#endif
