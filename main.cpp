#include "options.h"
#include "session.h"
#include "timer.h"

#include <csignal>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <sstream>
#include <unistd.h>

using namespace std;

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
    cout << "peak memory: " << get_peak_memory_in_kb() << " KB" << endl;
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

void print_help() {
    cout << "Usage:";
    cout << "--help,-h: print this help message" << endl;
    cout << "--number,-n: number of games for the session, preferably a number divisible by 4 (if smaller than 4, compulsory solos are disabled)" << endl;
    cout << "--compulsory-solo: play with compulsory solo (default: false)" << endl;
    cout << "--random,--r: use random cards for the whole session (default: false). if option is not set, you will be asked to input a card distribution manually (or to deal random cards) after each game";
    cout << "--seed,--s: random seed for random cards dealing" << endl;
    cout << "--announcing-version: 0 for the version in which a player asked for an announcement can choose between all the legal announcements and 1 for the version in which a player asked for an announcement only can opt to not announce or to announce the 'next' announcement for his team and then gets asked again immediately if he wants to make another announcement or not" << endl;
    cout << "--players,--p: specify four player types from { uct, human, random } (default: uct random random random)" << endl;
    cout << "--print-player-options: prints options that can be specified by using the --p*-options arguments of the program, then terminating" << endl;
    cout << "--p0-options: specify options for player 0 (only if UCT player; see --print-player-options for available options)" << endl;
    cout << "--p1-options: specify options for player 1 (only if UCT player; see --print-player-options for available options)" << endl;
    cout << "--p2-options: specify options for player 2 (only if UCT player; see --print-player-options for available options)" << endl;
    cout << "--p3-options: specify options for player 3 (only if UCT player; see --print-player-options for available options)" << endl;
    cout << "--create-graph: create a .dot-file for each constructed UCT tree (default: false)" << endl;
    cout << "--verbose,--v: display detailed output during play (default: false). recommended to enable if playing with human players" << endl;
    cout << "--uct-verbose: display detailed output from UCT players and UCT algorithm (default: false). only relevant if there is at least one UCT player" << endl;
    cout << "--debug,--d: display debug output in BeliefGameState (default: false)" << endl;
    cout << "--uct-debug: display debug output in Uct (default: false)" << endl;
}

void print_player_options() {
    string player_options = "(currently only a UCT player accepts options)\n\nversion:\n0 for an UCT algorithm with a number of simulations, each with a fixed card assignment and a number of rollouts per simulation, 1 for an UCT algorithm with a number of rollouts, each using a different card assignment\n\nscore points factor:\ninteger which score points get multiplyed by in order to obtain UCT rewards\n\nplayer's or team's points:\n0 for using player's point as an additional bias to the score points, 1 for using the player's team points\n\nplaying points divisor:\ninterger which the player's or the team points of the player get divided by before being added to the (modified) score points\n\nexploration:\ninteger used as exploration constant in the UCT formula\n\nrollouts:\ninteger setting the number of rollouts performed in a UCT search (either in total, or per simulation)\n\nsimulations:\ninteger setting the number of simulations performed in a UCT search, specify anything if using version 1 (do not leave empty though!)\n\nannouncements:\n0 to forbid the UCT player to do announcements, 1 to allow, 2 to allow but to forbid if all possible moves yield a negative reward\n\nWrong UCT formula:\n0 to use the correct UCT formula and 1 to use the total number of visits in the tree (i.e. the current number of rollout) rather than the number of total visits of the specific node for which the formula is calculated\n\nMC simulation:\n0 if no MC simulation should be carried on but all states encountered during a rollout should be added to the tree, i.e. more than one per rollout. 1 if a MC simulation should be carried on as soon as a leaf node was added to the tree, i.e. only one node is added to the tree per rollout\n\nAction selection:\n0 to choose the first successor when expanding the first node and use random action selection after a new node was inserted, 1 to also use random action selection when expanding the first node (rest same as 0), 2 to choose the first successor when expanding the first node and use heurstic guided action selection after a new node was inserted, 3 to use random action selection when expanding the first node and heristic guided action selection after a new node was inserted, 4 to use heuristic guided action whenever a successor needs to be chosen\n\n(defaults: 1 500 1 1 20000 1000 10 2 0 0 0)";
    cout << player_options << endl;
}

int get_int_option(int argc, char *argv[], int &index) {
    if (index + 1 >= argc) {
        cerr << "Missing (integer) argument after " << argv[index] << endl;
        exit(2);
    }
    int result = atoi(argv[index + 1]);
    ++index;
    return result;
}

void parse_players_options(int argc, char *argv[], int &index, vector<int> &players_options) {
    if (index + 11 >= argc) {
        cerr << "Missing eleven (integer) arguments after " << argv[index] << endl;
        exit(2);
    }
    players_options.reserve(11);
    for (int j = 0; j < 11; ++j) {
        int option = atoi(argv[index + 1 + j]);
        players_options.push_back(option);
    }
    index += 11;
}

void check_uct_player_options(const vector<int> &player_options) {
    if (player_options.size() != 11) {
        cerr << "must specify 11 player options" << endl;
        exit(2);
    }
    if (player_options[0] != 0 && player_options[0] != 1) {
        cerr << "version must be set to 0 or 1" << endl;
        exit(2);
    }
    if (player_options[1] < 1) {
        cerr << "score points factor must be greater 0" << endl;
        exit(2);
    }
    if (player_options[2] != 0 && player_options[2] != 1) {
        cerr << "using player's or team's points must be set to 0 or 1" << endl;
        exit(2);
    }
    if (player_options[3] < 1) {
        cerr << "playing points divisor must be greater 0" << endl;
        exit(2);
    }
    if (player_options[4] < 1) {
        cerr << "exploration constant must be greater 0" << endl;
        exit(2);
    }
    if (player_options[5] < 1) {
        cerr << "number of rollouts must be greater 0" << endl;
        exit(2);
    }
    if (player_options[6] < 1) {
        cerr << "number of simulations must be greater 0" << endl;
        exit(2);
    }
    if (player_options[7] != 0 && player_options[7] != 1 && player_options[7] != 2) {
        cerr << "announcement option must be set to 0, 1 or 2" << endl;
        exit(2);
    }
    if (player_options[8] != 0 && player_options[8] != 1) {
        cerr << "using wrong UCT formula must be set to 0 or 1" << endl;
        exit(2);
    }
    if (player_options[9] != 0 && player_options[9] != 1) {
        cerr << "using MC simulation must be set to 0 or 1" << endl;
        exit(2);
    }
    if (player_options[10] < 0 || player_options[10] > 4) {
        cerr << "action selection must be in the interval [0,4]" << endl;
        exit(2);
    }
}

int main(int argc, char *argv[]) {
    register_event_handlers();

    int number = 1000;
    bool compulsory_solo = false;
    bool random = false;
    int seed = 2012;
    int announcing_version = 1;
    vector<player_t> players_types;
    vector<vector<int> > players_options(4);
    bool create_graph = false;
    bool verbose = false;
    bool uct_verbose = false;
    bool debug = false;
    bool uct_debug = false;

    // TODO: test if important command line arguments trigger errors as intended
    // TODO: move parsing to Options? Or have its own class
    // TODO: even catch more invalid command line options

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            print_help();
            exit(0);
        } else if (arg == "--print-player-options") {
            print_player_options();
            exit(0);
        }
        else if (arg == "--number" || arg == "-n") {
            number = get_int_option(argc, argv, i);
            if (number % 4 != 0) {
                cerr << "number of games must be a multiple of 4" << endl;
                exit(2);
            }
        } else if (arg == "--compulsory-solo") {
            compulsory_solo = true;
        } else if (arg == "--random" || arg == "r") {
            random = true;
        } else if (arg == "--seed" || arg == "s") {
            seed = get_int_option(argc, argv, i);
        } else if (arg == "--announcing-version") {
            announcing_version = get_int_option(argc, argv, i);
        } else if (arg == "--players" || arg == "p") {
            if (i + 4 >= argc) {
                cerr << "Missing four arguments after " << argv[i] << endl;
                exit(2);
            }
            players_types.reserve(4);
            for (int j = 0; j < 4; ++j) {
                string player_type(argv[i + 1 + j]);
                if (player_type == "human") {
                    players_types.push_back(HUMAN);
                } else if (player_type == "random") {
                    players_types.push_back(RANDOM);
                } else if (player_type == "uct") {
                    players_types.push_back(UCT);
                } else {
                    cerr << "Players types can be human, random or uct" << endl;
                    exit(2);
                }
            }
            i += 4;
        } else if (arg == "--p0-options") {
            parse_players_options(argc, argv, i, players_options[0]);
        } else if (arg == "--p1-options") {
            parse_players_options(argc, argv, i, players_options[1]);
        } else if (arg == "--p2-options") {
            parse_players_options(argc, argv, i, players_options[2]);
        } else if (arg == "--p3-options") {
            parse_players_options(argc, argv, i, players_options[3]);
        } else if (arg == "--create-graph") {
            create_graph = true;
        } else if (arg == "--verbose" || arg == "v") {
            verbose = true;
        } else if (arg == "--uct-verbose") {
            uct_verbose = true;
        } else if (arg == "--debug" || arg == "d") {
            debug = true;
        } else if (arg == "--uct-debug") {
            uct_debug = true;
        } else {
            cerr << "Unrecognized option " << arg << endl;
            exit(2);
        }
    }

    // TODO: use better player options (ideally named)

    if (players_types.empty()) {
        // use default values
        players_types.push_back(UCT);
        players_types.push_back(RANDOM);
        players_types.push_back(RANDOM);
        players_types.push_back(RANDOM);
    }
    for (size_t i = 0; i < players_options.size(); ++i) {
        if (players_types[i] != UCT && !players_options[i].empty()) {
            cerr << "Specifying player options for player " << i
                 << " only possible if it is a UCT player" << endl;
            exit(2);
        }
        if (players_types[i] == UCT) {
            if (players_options[i].empty()) {
                // use default values
                players_options[i].push_back(1);
                players_options[i].push_back(500);
                players_options[i].push_back(1);
                players_options[i].push_back(1);
                players_options[i].push_back(20000);
                players_options[i].push_back(1000);
                players_options[i].push_back(10);
                players_options[i].push_back(2);
                players_options[i].push_back(0);
                players_options[i].push_back(0);
                players_options[i].push_back(0);
            } else {
                check_uct_player_options(players_options[i]);
            }
        }
    }

    Timer timer;
    Options options(number, compulsory_solo, players_types, random, seed,
                    verbose, uct_verbose, debug, uct_debug, players_options,
                    create_graph, announcing_version);
    Session session(options);
    cout << "time: " << timer << "s" << endl;
    return 0;
}
