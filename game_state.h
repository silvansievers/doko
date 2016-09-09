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

#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "cards.h"
#include "move.h"
#include "trick.h"

#include <cassert>
#include <vector>

/**
The following will informally explain the Doppelkopf rules concerning a session (with solos), game type determination, announcing and scoring. The explanation of the game play itself (i.e. playing correct suits, which game type has which trump suits, the meaning of announcements etc.) will not be explained.
All rules are implemented according to the official tourney rules settled by the Deutscher Doppelkopf Verband (www.doko-verband.de).

Session/solos:
A standard session consists of 24 games during which every player has to play at least one solo, a so called compulsory solo, where he has the right to be the player in first position, i.e. the player playing the first card. These games get "repeated" in the sense that the same dealer deals cards again, such that the player who normally would have started playing still is the first player to start next game. If at the end, the number of players that still must play a solo equals the remaining number of games, players are forced to play solo (a so called "vorfuehrung"). These games are not "repeated" (this does not matter really, as starting players are fixed at this moment and for the program, it does not matter who is the "dealer"). A silent solo (having both queens, but not announcing a solo play) and a failed marriage (no other player than the marriage player himself made one of the first three tricks) do not count as compulsory solo but as a regular solo (important for scoring).

Game type determination:
Before the actual game starts, the players must find out what type of game they are going to play - a regular game, a marriage or any kind of solo. To do so, starting with the first positioned player, players must say "healthy" if they do not want to play a solo nor have a marriage. If they do want to do so, they claim to have "reservation". If there is just one player with a reservation, this player may play a marriage or a solo. If there are several players with a reservation, then no marriage can be played because a solo always has higher priority. For solos, one must distinguish two kind of solos: a compulsory solo (meaning the first solo of the session for the player) or a lust solo (any solo after the compulsory solo, i.e. the player already played a solo (see session/solos above!)). A compulsory solo always has higher priority over a lust solo. If all priorities are equal, i.e. two players want to play a compulsory or a lust solo, then the one positioned ahead of the other has the right to play it. It is always possible for a player who knows that he has the highest priority with his announcement to immediately announce his solo without waiting for the other players to say healthy or to claim a reservation.

Announcing:
The number of cards on a player's hand determine if he is still allowed to make an announcement:
 - Re or Kontra with 11 cards left
 - No 90 with 10 cards left
 - No 60 with 9 cards left
 - No 30 with 8 cards left
 - Black with 7 cards left
For a marriage, announcements are not possible until the teams are clear, i.e. either until the marriage player marries a player or after the third trick is completed without the marriage having actually happened. If the first trick is the "clarification trick", the rules above are not changed. If the clarification trick is the second (third) one, players may have two (three) fewer cards than denoted above.
There is another exception concerning a reply (only re or kontra) to an announcement of the other team. In this case, saying re or kontra is also allowed with less than 11 cards. A reply is possible not later than holding one card less than the other's team announcement required number of cards. Example: re announces no 30 (no matter at which moment - at the start or with 8 cards left) and kontra now decides to say kontra because they think they will reach 30. They may say kontra at the latest when they hold 7 cards (because re was allowed to say no 30 as long as they hold 8 cards).

Scoring:
Re generally wins with 121 playing points (i.e. card value points made during the game) if they did not announce more than re, kontra generally wins with 120 points, with the exception of the only announcement made was kontra (especially no re, no no 90 and more) where also kontra needs 121 points to win (and i.e. re wins with only 120 points).
If a team announces no 90 (no 60, no 30)/black, it wins if the opposing team has less than 90 (60, 30) points/wins no trick. If a team did not announce more than re or kontra,it also wins by reaching 90 (60, 30) points/making a trick against an announcement of no 90 (no 60, no 30)/black of the other team.
It can happen that both teams do not win, i.e. when both teams announce at least no 90 and both teams fail in reaching 151 points.

Teams get the following score points after a game (note the difference between points made during the game and score points which evaluate a finished game and which are summed up over a whole session of games):

 - Score points that are always granted to either the winning team if there is one or to both teams if no team won (teams can get several points of this type):
   +1 for the team reaching at least 120 (90, 60, 30) points against an announcement of no 90 (no 60, no 30, black) of the other team (thus up to 4 points, e.g. 2 points for reaching at least 90 points vs an announcement of no 30)
   +1 for the team reaching at least 151 (181, 211) points/winning all tricks (i.e. the other team has less than 90 (60, 30) points/is black) (thus up to 4 points, e.g. 2 points for reaching at least 181 points)

 - Score points that are "winner's" points and thus only granted to the winning team:
   +1 for a won game
   +1 for each announcement of no 90, no 60, no 30 and black (of both teams!, i.e. theoretically up to 8 points)
   +2 for re and kontra each (i.e. up to 4)

 - Special score points that are only granted in non-solo games, i.e. in regular and successful marriage games and *not* in solo games. These points are always granted to both teams, no matter if they won or not (it may even happen that the winning team gets minus points because the losing team has more special points):
   +1 for winning against the re team (because they have the higher trumps)
   +1 for catching a fox (diamonds ace) of the other team (there are two foxes, so up to 2 points possible)
   +1 for winning the last trick with a charlie (clubs jack)
   +1 for each trick containing at least 40 points (a so called doppelkopf)

The above points are added up for each team. The team with more points gets the difference of points between the teams as a positive, the other team as a negative value. In the case of a solo game, the solo players points gets multiplied by 3 before writing it down/storing it. Each player of a team gets his team's points. Players' points should always add up to 0 during a session.
*/

class GameType;
class Options;

/**
GameState is the base class of BeliefGameState and ActualGameState and is never being used on its own. GameState provides functionality that both derived classes can use, i.e. "game rules" such as which game type is being played, what legal moves can a player do and game evaluation after termination. For the rest of functionality, see the afore-mentioned sub-classes.

A general note on the internal representation:
 - a player is represented by its unique number, ranging from 0 to 3
 - the teams are denoted by an integer value: 1 means re team, 0 means kontra team, -1 means uninitialized/not known. this is especially important for all datastructures that store some properties for a team (or both); these are always accessed by index 0 for the kontra team and by index 1 for the re team (for example: announcement_t announcements[2]). At some places also boolean values for the teams are used (e.g. if a parameter needs to denote whether a player is part of the re team or not).
*/

class GameState {
protected:
    const Options &options;
    Cards cards[4];
    const GameType *game_type;
    int players_team[4]; // saves for each player whether he is part of the re team (1) or the kontra team (0) (uninitialized: -1)
    std::vector<Trick> tricks; // all tricks that have been played
    bool session_instance; // to know if players' teams need to be assigned (or asserted because it is already known or because the session instance of gamestate knows the true card distribution) when somebody announces something or plays a queen of clubs
    int player_played_queen_of_clubs; // initialized: -1, after the first queen of clubs was played, stores the player who played it. needed to find out if the same plays the second queen of clubs too, in which case all other players must be kontra players (regular game only)

    // fields related to announcements
    int players_latest_moment_for_announcement[4]; // number of cards a player must hold to allow for announcements
    announcement_t announcements[2]; // team anouncements (only the highest one)
    bool first_announcement_in_time[2]; // true iff the first announcement was a 'real' announcement and not a reply to an announcement from the other team
    int card_number_for_latest_possible_reply[2]; // stores how many cards each team has to have making a reply to an announcement
    int players_known_team[4]; // only difference to players_team is that for a regular game, this gets initialized with -1 and updated later to 0 or 1 as soon as all players participating in the game have the information about that player's team
    bool teams_are_known; // from the moment on where both teams and their players are identified, this variable is set to true

    // fields related to the special cases marriage and solo
    int solo_or_marriage_player; // = -1 if no solo or marriage play
    bool compulsory_solo; // compulsory_solo = true implies solo_or_marriage_player != -1
    int number_of_clarification_trick; // for adapting announcements rules in the case of marriage

    // during the game: game type determination, announcements, card moves
    int corrected_number_of_cards(int number) const; // adapts the number of cards needed for announcements when game type is marriage
    /* announcement_possible does NOT check for players which team they belong to in order to avoid not asking a player for an announcement when the other players cannot know that this player is not allowed for any announcement, thus telling them informations they normally would not have. (player's) cards_count was introduced as a parameter for the UctPlayer-usage of this class. */
    bool announcement_possible(int player, int cards_count) const;
    bool announcement_legal(int player, announcement_t announcement, int cards_count) const;
    int get_number_of_cards_for_announcement(announcement_t announcement) const;
    announcement_t get_announcement_for_number_of_cards(int number_of_cards) const; // returns the minimum announcement required in order for the given number of cards to be a correct limit for announcements
    void check_teams_are_known(); // this method tests if (with a given announcement) now the teams are uniquely determined. if yes, it also updates the latest moments for announcing for each player if necessary
    void set_announcement(int player, announcement_t announcement, bool is_re_player, int cards_count);
    void set_latest_moment_for_announcements(); // only called in case of marriage after the clearification trick
    void get_legal_announcements_for_player(int player, std::vector<Move> &legal_announcements) const;
    int update(int player, Card card); // set the card played by splayer and returns the player who is next
    void assign_solo_player_to_re_team(int player);

    // after game end: points calculation
    /* computes all points and special points made during the game (iterates over all tricks). also computes whether one team was played black or not. the argument count_re_players serves to determine whether special points must be computed or not. */
    void get_points_and_special_points(int points[4], int &special_points_for_re, bool black[2], int count_re_players) const;
    bool has_team_lost(bool re_team, int points, const bool black[2]) const;
    int get_team_score_points(bool re_team, int points, const bool black[2]) const; // computes score points that will always be distributed, independent of who (or if one team) won. therefore called for both teams.
    int get_winning_team_score_points(bool re_team, int points, const bool black[2]) const; // computes score points that will be granted for the winning team only
public:
    GameState(const Options &options, bool session_instance);
    bool game_finished() const; // changed to public for Uct
    void get_score_points(int *players_score_points, int *players_points = 0, int *team_points = 0) const; // computes the final score points made by each player. players_points needed by Uct to not only get the score points, but also the points players made during the game. furthermore, if player_points = 0 (the case when Session uses GameState), the score points etc are printed to standard out
    bool is_compulsory_solo() const {
        return compulsory_solo;
    }
    int get_compulsory_solo_player() const { // needed by Session to store which player already played his compulsory solo
        assert(is_compulsory_solo() && solo_or_marriage_player != -1);
        return solo_or_marriage_player;
    }
    int get_players_team(int player) const { // needed by Uct to compute team points
        assert(players_team[player] != -1);
        assert(players_known_team[player] != -1);
        return players_team[player];
    }
};

//extern bool _has_team_lost(const announcement_t announcements[2], bool re_team, int points, const bool black[2]);

#endif
