#include "cards.h"
#include "options.h"
#include "session.h"

#include <boost/program_options.hpp>
#include <cassert>
#include <csignal>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace boost::program_options;

int get_peak_memory_in_kb() {
    // On error, produces a warning on cerr and returns -1.
    int memory_in_kb = -1;

    ostringstream filename_stream;
    filename_stream << "/proc/" << getpid() << "/status";
    const char *filename = filename_stream.str().c_str();

    ifstream procfile(filename);
    string word;
    while (procfile.good()) {
        procfile >> word;
        if (word == "VmPeak:") {
            procfile >> memory_in_kb;
            break;
        }
        // Skip to end of line.
        procfile.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    if (procfile.fail())
        memory_in_kb = -1;

    if (memory_in_kb == -1)
        cerr << "warning: could not determine peak memory" << endl;
    return memory_in_kb;
}

void print_peak_memory() {
    cout << "\npeak memory: " << get_peak_memory_in_kb() << " KB" << endl;
}

void signal_handler(int signal_number) {
    // See glibc manual: "Handlers That Terminate the Process"
    static volatile sig_atomic_t handler_in_progress = 0;
    if (handler_in_progress)
        raise(signal_number);
    handler_in_progress = 1;
    print_peak_memory();
    cout << "caught signal " << signal_number << " -- exiting" << endl;
    signal(signal_number, SIG_DFL);
    raise(signal_number);
}

void exit_handler(int, void *) {
    print_peak_memory();
}

void register_event_handlers() {
    // On exit or when receiving certain signals such as SIGINT (Ctrl-C),
    // print the peak memory usage.
    on_exit(exit_handler, 0);
    signal(SIGABRT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGSEGV, signal_handler);
    signal(SIGINT, signal_handler);
}

int check_uct_player_options(vector<int> &player_options) {
    if (player_options.empty()) { // use default options
         player_options.push_back(1);
         player_options.push_back(500);
         player_options.push_back(1);
         player_options.push_back(1);
         player_options.push_back(20000);
         player_options.push_back(1000);
         player_options.push_back(10);
         player_options.push_back(2);
         player_options.push_back(0);
         player_options.push_back(0);
         player_options.push_back(0);
         return 0;
    } else {
        if (player_options.size() != 11) {
            cerr << "must specify 11 player options" << endl;
            return 2;
        }
        if (player_options[0] != 0 && player_options[0] != 1) {
            cerr << "version must be set to 0 or 1" << endl;
            return 2;
        }
        if (player_options[1] < 1) {
            cerr << "score points factor must be greater 0" << endl;
            return 2;
        }
        if (player_options[2] != 0 && player_options[2] != 1) {
            cerr << "using player's or team's points must be set to 0 or 1" << endl;
            return 2;
        }
        if (player_options[3] < 1) {
            cerr << "playing points divisor must be greater 0" << endl;
            return 2;
        }
        if (player_options[4] < 1) {
            cerr << "exploration constant must be greater 0" << endl;
            return 2;
        }
        if (player_options[5] < 1) {
            cerr << "number of rollouts must be greater 0" << endl;
            return 2;
        }
        if (player_options[6] < 1) {
            cerr << "number of simulations must be greater 0" << endl;
            return 2;
        }
        if (player_options[7] != 0 && player_options[7] != 1 && player_options[7] != 2) {
            cerr << "announcement option must be set to 0, 1 or 2" << endl;
            return 2;
        }
        if (player_options[8] != 0 && player_options[8] != 1) {
            cerr << "using wrong UCT formula must be set to 0 or 1" << endl;
            return 2;
        }
        if (player_options[9] != 0 && player_options[9] != 1) {
            cerr << "using MC simulation must be set to 0 or 1" << endl;
            return 2;
        }
        if (player_options[10] < 0 || player_options[10] > 4) {
            cerr << "action selection must be in the interval [0,4]" << endl;
            return 2;
        }
        return 0;
    }
}

int main(int argc, char *argv[]) {
    register_event_handlers();
    int number;
    bool compulsory_solo = false;
    bool random = false;
    int seed;
    int announcing_version;
    vector<string> players;
    player_t player_default[4] = { UCT, RANDOM, RANDOM, RANDOM };
    vector<vector<int> > players_options(4);
    string player_options = "(currently only a UCT player accepts options)\n\nversion:\n0 for an UCT algorithm with a number of simulations, each with a fixed card assignment and a number of rollouts per simulation, 1 for an UCT algorithm with a number of rollouts, each using a different card assignment\n\nscore points factor:\ninteger which score points get multiplyed by in order to obtain UCT rewards\n\nplayer's or team's points:\n0 for using player's point as an additional bias to the score points, 1 for using the player's team points\n\nplaying points divisor:\ninterger which the player's or the team points of the player get divided by before being added to the (modified) score points\n\nexploration:\ninteger used as exploration constant in the UCT formula\n\nrollouts:\ninteger setting the number of rollouts performed in a UCT search (either in total, or per simulation)\n\nsimulations:\ninteger setting the number of simulations performed in a UCT search, specify anything if using version 1 (do not leave empty though!)\n\nannouncements:\n0 to forbid the UCT player to do announcements, 1 to allow, 2 to allow but to forbid if all possible moves yield a negative reward\n\nWrong UCT formula:\n0 to use the correct UCT formula and 1 to use the total number of visits in the tree (i.e. the current number of rollout) rather than the number of total visits of the specific node for which the formula is calculated\n\nMC simulation:\n0 if no MC simulation should be carried on but all states encountered during a rollout should be added to the tree, i.e. more than one per rollout. 1 if a MC simulation should be carried on as soon as a leaf node was added to the tree, i.e. only one node is added to the tree per rollout\n\nAction selection:\n0 to choose the first successor when expanding the first node and use random action selection after a new node was inserted, 1 to also use random action selection when expanding the first node (rest same as 0), 2 to choose the first successor when expanding the first node and use heurstic guided action selection after a new node was inserted, 3 to use random action selection when expanding the first node and heristic guided action selection after a new node was inserted, 4 to use heuristic guided action whenever a successor needs to be chosen\n\n(defaults: 1 500 1 1 20000 1000 10 2 0 0 0)";
    bool create_graph = false;
    bool verbose = false;
    bool uct_verbose = false;
    bool debug = false;
    bool uct_debug = false;

    options_description desc("Allowed options");
    desc.add_options()
    ("help,h", "print this help message")
    ("number,n", value<int>(&number)->default_value(1000), "number of games for the session, preferably a number divisible by 4 (if smaller than 4, compulsory solos are disabled)")
    ("compulsory-solo", "play with compulsory solo (default: false)")
    ("random,r", "use random cards for the whole session (default: false). if option is not set, you will be asked to input a card distribution manually (or to deal random cards) after each game")
    ("seed,s", value<int>(&seed)->default_value(2012), "random seed for random cards dealing")
    ("announcing-version", value<int>(&announcing_version)->default_value(1), "0 for the version in which a player asked for an announcement can choose between all the legal announcements and 1 for the version in which a player asked for an announcement only can opt to not announce or to announce the 'next' announcement for his team and then gets asked again immediately if he wants to make another announcement or not")
    ("players,p", value<vector<string> >(&players)->multitoken(), "specify four player types from { uct, human, random } (default: uct random random random)")
    ("print-player-options", "prints options that can be specified by using the --p*-options arguments of the program, then terminating")
    ("p0-options", value<vector<int> >(&players_options[0])->multitoken(), "specify options for player 0 (see --print-player-options for available options)")
    ("p1-options", value<vector<int> >(&players_options[1])->multitoken(), "specify options for player 1 (see --print-player-options for available options)")
    ("p2-options", value<vector<int> >(&players_options[2])->multitoken(), "specify options for player 2 (see --print-player-options for available options)")
    ("p3-options", value<vector<int> >(&players_options[3])->multitoken(), "specify options for player 3 (see --print-player-options for available options)")
    ("create-graph", "create a .dot-file for each constructed UCT tree (default: false)")
    ("verbose,v", "display detailed output during play (default: false). recommended to enable if playing with human players")
    ("uct-verbose", "display detailed output from UCT players and UCT algorithm (default: false). only relevant if there is at least one UCT player")
    ("debug,d", "display debug output in BeliefGameState (default: false)")
    ("uct-debug", "display debug output in Uct (default: false)")
    ;

    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    vector<player_t> players_types;
    players_types.reserve(4);
    if (vm.count("help")) {
        cout << desc << endl;
        return 0;
    }
    if (vm.count("number")) {
        if (number < 1) {
            cerr << "number of games must be at least 1" << endl;
            return 2;
        }
    }
    if (vm.count("compulsory-solo")) {
        if (number >= 4)
            compulsory_solo = true;
    }

    if (vm.count("random")) {
        random = true;
    }
    if (vm.count("seed")) {
        if (seed < 0 || seed > numeric_limits<int>::max()) {
            cerr << "seed should be in the range [0..2^31-1]" << endl;
            return 2;
        }
    }
    if (vm.count("players")) { // players were manually specified
        if (players.size() != 4) {
            cerr << "you must specify exactly 4 player types" << endl;
            return 2;
        }
        for (size_t i = 0; i < players.size(); ++i) {
            if (players[i] == "uct")
                players_types.push_back(UCT);
            else if (players[i] == "human")
                players_types.push_back(HUMAN);
            else if (players[i] == "random")
                players_types.push_back(RANDOM);
            else {
                cerr << "unknown input for player type" << endl;
                return 2;
            }
        }
    } else { // use default player types
        players_types = vector<player_t>(player_default, player_default + sizeof(player_default) / sizeof(player_t));
    }
    // in both cases, need to iterate over player types to check if there is a UCT player and if so, to check its configuration or to set the default configuration
    for (size_t i = 0; i < players_types.size(); ++i) {
        if (players_types[i] == UCT) {
            if (check_uct_player_options(players_options[i]) == 2)
                return 2;
        }
    }
    if (vm.count("print-player-options")) {
        cout << player_options << endl;
        return 0;
    }
    if (vm.count("create-graph")) {
        create_graph = true;
    }
    if (vm.count("verbose")) {
        verbose = true;
    }
    if (vm.count("uct-verbose")) {
        uct_verbose = true;
    }
    if (vm.count("debug")) {
        debug = true;
    }
    if (vm.count("uct-debug")) {
        uct_debug = true;
    }

    Options options(number, compulsory_solo, players_types, random, seed, verbose, uct_verbose, debug, uct_debug,
                    players_options, create_graph, announcing_version);
    Session session(options);
    return 0;
}
