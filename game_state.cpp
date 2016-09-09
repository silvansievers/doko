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

#include "game_state.h"

#include "game_type.h"
#include "options.h"

#include <iostream>

using namespace std;

GameState::GameState(const Options &options_, bool session_instance_)
    : options(options_), game_type(0), session_instance(session_instance_), player_played_queen_of_clubs(-1),
    teams_are_known(false), solo_or_marriage_player(-1), compulsory_solo(false), number_of_clarification_trick(-1) {
    for (int i = 0; i < 4; ++i) {
        players_team[i] = -1;
        players_latest_moment_for_announcement[i] = 11;
        players_known_team[i] = -1;
    }
    tricks.reserve(12);
    for (int i = 0; i < 2; ++i) {
        announcements[i] = NONE;
        first_announcement_in_time[i] = false;
        card_number_for_latest_possible_reply[i] = -1;
    }
}

bool GameState::game_finished() const {
    return (tricks.size() == 12 && tricks.back().completed());
}

int GameState::corrected_number_of_cards(int number) const {
    if (*game_type == marriage) {
        assert(number_of_clarification_trick != -1); // corrected_number_of_cards should first be called after the clarification trick, otherwise no announcemnets are possible
        return number - number_of_clarification_trick;
    }
    return number;
}

bool GameState::announcement_possible(int player, int cards_count) const {
    if (*game_type == marriage && number_of_clarification_trick == -1)
        return false;
    if (cards_count >= players_latest_moment_for_announcement[player])
        return true;
    else
        return false;
}

bool GameState::announcement_legal(int player, announcement_t announcement, int cards_count) const {
    assert(announcement_possible(player, cards_count));
    assert(players_team[player] != -1);
    bool is_re_player = players_team[player];
    // repeated announcements are forbidden
    if (announcement <= announcements[is_re_player])
        return false;
    // having 11 (or less in case of marriage) cards always allows for all announcements
    if (cards_count >= corrected_number_of_cards(11))
        return true;
    // only if the team announced re or kontra while having 11 or more cards, players from that team may do further announcements
    if (first_announcement_in_time[is_re_player]) {
        announcement_t team_announcement_so_far = announcements[is_re_player];
        // having at least 10 (or less in case of marriage) cards and having said at least re or kontra (i.e. no 90, 60 is "more" than re or kontra, implying them) allows for more announcements
        if (cards_count >= corrected_number_of_cards(10) && team_announcement_so_far >= REKON)
            return true;
        if (cards_count >= corrected_number_of_cards(9) && team_announcement_so_far >= N90)
            return true;
        if (cards_count >= corrected_number_of_cards(8) && team_announcement_so_far >= N60)
            return true;
        if (cards_count >= corrected_number_of_cards(7) && team_announcement_so_far >= N30)
            return true;
    }
    // otherwise only a reply (i.e. re or kontra) by the opposing team is allowed
    if (!first_announcement_in_time[is_re_player]) {
        // cards_count < 11 catches the case where no announcement has been made by the team yet (i.e. first_announcement_in_time is false for that team), but they still have more than 10 cards and therefore are allowed to announce anything
        if (cards_count < corrected_number_of_cards(11) && cards_count >= card_number_for_latest_possible_reply[is_re_player]) {
            if (announcement == REKON)
                return true;
            else
                return false;
        }
    }
    return false;
}

int GameState::get_number_of_cards_for_announcement(announcement_t announcement) const {
    switch (announcement) {
        case NONE:
            return corrected_number_of_cards(11);
        case REKON:
            return corrected_number_of_cards(10);
        case N90:
            return corrected_number_of_cards(9);
        case N60:
            return corrected_number_of_cards(8);
        case N30:
            return corrected_number_of_cards(7);
        case SCHWARZ:
            return corrected_number_of_cards(6);
    }
    assert(false);
    return 0;
}

announcement_t GameState::get_announcement_for_number_of_cards(int number_of_cards) const {
    if (number_of_cards == 12 || number_of_cards == 13)
        return NONE;
    if (number_of_cards == corrected_number_of_cards(11))
        return NONE;
    if (number_of_cards == corrected_number_of_cards(10))
        return REKON;
    if (number_of_cards == corrected_number_of_cards(9))
        return N90;
    if (number_of_cards == corrected_number_of_cards(8))
        return N60;
    if (number_of_cards == corrected_number_of_cards(7))
        return N30;
    if (number_of_cards == corrected_number_of_cards(6))
        return SCHWARZ;
    assert(false);
    return NONE;
}

void GameState::check_teams_are_known() {
    assert(!teams_are_known);
    int re_players_known = 0;
    int kontra_players_known = 0;
    for (int i = 0; i < 4; ++i) {
        //cout << "player " << i << "'s known team: " << players_known_team[i] << endl;
        if (players_known_team[i] == 1)
            ++re_players_known;
        else if (players_known_team[i] == 0)
            ++kontra_players_known;
    }
    // check if there are 2 re playeres or 3 kontra players, then the teams are determined. note that one needs to know 3 kontra players because it is always possible that one player got both queens of clubs. for the re team it is enough to know two players of them
    if (re_players_known == 2 || kontra_players_known == 3) {
        // this will be done exactly once during a game
        teams_are_known = true;
        for (int i = 0; i < 4; ++i) {
            //if (options.get_debug())
                //cout << "player " << i << "'s known team: " << players_known_team[i] << endl;
            if (re_players_known == 2 && players_known_team[i] == -1)
                players_known_team[i] = 0;
            else if (kontra_players_known == 3 && players_known_team[i] == -1)
                players_known_team[i] = 1;
            assert(players_known_team[i] != -1);
            //if (options.get_debug())
                //cout << "player " << i << "'s team: " << players_team[i] << " players known team: " << players_known_team[i] << endl;
            if (session_instance) {
                assert(players_team[i] == players_known_team[i]);
            } else { // UctPlayer's instance
                if (players_team[i] != players_known_team[i]) {
                    assert(players_team[i] == -1);
                    //cout << "need to update actual teams according to known teams!" << endl;
                    players_team[i] = players_known_team[i];
                }
            }
            assert(players_team[i] != -1);
        }
        // now that the teams are known, maybe some of the announcement possibilites for players have to be updated
        for (int i = 0; i < 4; ++i) {
            bool is_re_player = players_team[i];
            if (announcements[is_re_player] < get_announcement_for_number_of_cards(players_latest_moment_for_announcement[i])) {
                // the own team did not announce at least <get_announcement_for_number_of_cards>, in which case the player would have time do a further announcement as long as he has <latest_moment_for_announcement> cards and thus nothing would need to be adjusted
                if (announcements[!is_re_player] < get_announcement_for_number_of_cards(players_latest_moment_for_announcement[i]) || announcements[is_re_player] >= REKON) {
                    // the other team did not announce at least <get_announcement_for_number_of_cards>, in which case the player would have time to reply as long as he has <latest_moment_for_announcement> cards and thus nothing would need to be adjusted
                    // OR
                    // the other team DID announce enough to allow for a reply for the current number, but the team already announced re or kontra (does not matter if as a reply or even in time), they cannot do further announcements (because if they could, the very first if-condition would have been fulfilled)
                    assert(players_latest_moment_for_announcement[i] <= get_number_of_cards_for_announcement(announcements[is_re_player])); // either the number is correctly set already and does not need to be adjusted or it is set too low because the other team is allowed to do announcements until a later moment, but the number should never be too high.
                    players_latest_moment_for_announcement[i] = get_number_of_cards_for_announcement(announcements[is_re_player]);
                }
            }
        }
    }
}

void GameState::set_announcement(int player, announcement_t announcement, bool is_re_player, int cards_count) {
    assert(announcement != NONE);
    if (!teams_are_known) {
        if (players_known_team[player] == -1) {
            players_known_team[player] = is_re_player;
            if (session_instance)
                assert(players_team[player] == is_re_player);
            else
                players_team[player] = is_re_player;
        } else { // player's team has been known before, thus players_team should be correctly set
            assert(players_known_team[player] == is_re_player);
            assert(players_team[player] == is_re_player);
        }
        check_teams_are_known();
    }
    if (session_instance)
        assert(announcement_legal(player, announcement, cards_count));
    // cannot assert announcement_legal for the case this is an UctPlayer's instance because players_team might not be correctly set!

    assert(announcements[is_re_player] < announcement);
    announcements[is_re_player] = announcement;
    if (cards_count >= corrected_number_of_cards(11))
        first_announcement_in_time[is_re_player] = true;
    int latest_moment_for_announcement = get_number_of_cards_for_announcement(announcement);
    // TODO: is there a way to simplify this loop (some of the elements are repetitive)? probably not in a simple way
    for (int i = 0; i < 4; ++i) {
        if (teams_are_known) { // teams are known: differentiate between the teams and update all players (except for the case of a reply or announcing black in which case only the announcing player's team needs to be updated)
            if (players_team[i] == is_re_player) { // update the players of the announcing team
                if (announcement == REKON && cards_count < corrected_number_of_cards(11)) {
                    assert(!first_announcement_in_time[is_re_player]);
                    players_latest_moment_for_announcement[i] = 13; // after a reply (re or kontra later than normally allowed), a team cannot do any further announcements (13 is used for the check below in order to see )
                } else if (announcement == SCHWARZ) {
                    players_latest_moment_for_announcement[i] = 13; // a team which announced black cannot do any further announcements
                } else {
                    if (latest_moment_for_announcement < players_latest_moment_for_announcement[i]) {
                        // contrarily to what one may think, even if the teams are known, it can happen that the player already has a latest moment for announcement which is lower than the announcement which he is just doing allows because the opposing team might already have done such a high announcement.
                        players_latest_moment_for_announcement[i] = latest_moment_for_announcement;
                    }
                }
            } else { // update the players of the other team
                if (latest_moment_for_announcement < players_latest_moment_for_announcement[i]) {
                    // the announcement allows for the other team to have a later "latest moment for announcement" because the team can always do a reply
                    if (players_latest_moment_for_announcement[i] != 13) {
                        // this extra check prevents that if a team announces black early on in the game and the other team then announces something, then the team who announced black before would suddenly be allowed to do announcements again (and thus always be asked for an announcement with just the option to say "no")
                        players_latest_moment_for_announcement[i] = latest_moment_for_announcement;
                    }
                }
            }
        } else if (!teams_are_known) { // teams are not known: differentiate between the announcer and all other players and update all players (except for the case of a reply or announcing black in which case only the announcing player needs to be updated)
            if (i == player) { // update the announcing player
                if (announcement == REKON && cards_count < corrected_number_of_cards(11)) {
                    assert(!first_announcement_in_time[is_re_player]);
                    players_latest_moment_for_announcement[i] = 13; // after a replay (re or kontra later than normally allowed), a player cannot do any further announcements
                } else if (announcement == SCHWARZ) {
                    players_latest_moment_for_announcement[i] = 13; // a player who announced black cannot do any further announcements
                } else {
                    if (latest_moment_for_announcement < players_latest_moment_for_announcement[i]) {
                        // same as a above
                        players_latest_moment_for_announcement[i] = latest_moment_for_announcement;
                    }
                }
            } else { // update all other players
                if (latest_moment_for_announcement < players_latest_moment_for_announcement[i]) {
                    // this is needed to prevent an announcement of re or kontra after an higher announcement of the other team to set the latest moment for all players back to a higher card number which would be wrong of course
                    if (players_latest_moment_for_announcement[i] != 13) {
                        // this extra check prevents that if a player announces black early on in the game and the other team then announces something, then the player who announced black before would suddenly be allowed to do announcements again (and thus always be asked for an announcement with just the option to say "no")
                        players_latest_moment_for_announcement[i] = latest_moment_for_announcement;
                    }
                }
            }
        }
    }
    card_number_for_latest_possible_reply[!is_re_player] = latest_moment_for_announcement;
}

void GameState::set_latest_moment_for_announcements() {
    for (int i = 0; i < 4; ++i) {
        players_latest_moment_for_announcement[i] = corrected_number_of_cards(11);
    }
}

void GameState::get_legal_announcements_for_player(int player, vector<Move> &legal_announcements) const {
    assert(players_team[player] != -1);
    bool is_re_player = players_team[player];
    legal_announcements.push_back(Move(NONE, is_re_player)); // need to do this because announcement_legal always returns false for NONE (which is semantically correct because NONE is de facto not an announcement, but still it is always possible to say nothing)
    for (int i = 1; i < NUM_ANNOUNCEMENTS; ++i) {
        announcement_t announcement = static_cast<announcement_t>(i);
        if (announcement_legal(player, announcement, cards[player].size())) {
            legal_announcements.push_back(Move(announcement, is_re_player));
            if (options.get_announcing_version() == 1)
                break; // only allow the "next" announcement to be possible and ask the same player again if he wants to announce more
        }
    }
}

int GameState::update(int player, Card card) {
    if (*game_type == regular && !teams_are_known) {
        if (card == CQ || card == CQ_) {
            if (players_known_team[player] == -1) {
                players_known_team[player] = 1;
                if (session_instance)
                    assert(players_team[player] == 1);
                else
                    players_team[player] = 1;
            } else { // player's team has been known before, thus players_team should be correctly set
                assert(players_known_team[player] == 1);
                assert(players_team[player] == 1);
            }
            if (player_played_queen_of_clubs == player) { // the same player already played the first queen of clubs, thus all other players now are known to be kontra
                int player_it = next_player(player);
                for (int i = 0; i < 3; ++i) {
                    if (players_known_team[player_it] != -1) {
                        assert(players_known_team[player_it] == 0);
                        assert(players_team[player_it] == 0);
                    } else {
                        players_known_team[player_it] = 0;
                        if (session_instance)
                            assert(players_team[player_it] == 0);
                        else
                            players_team[player_it] = 0;
                    }
                    player_it = next_player(player_it);
                }
            }
            player_played_queen_of_clubs = player;
            check_teams_are_known();
        }
    }
    cards[player].remove_card(card);
    tricks.back().set_card(player, card);
    if (tricks.back().completed() && tricks.size() != 12) {
        player = tricks.back().taken_by();
        if (*game_type == marriage && number_of_clarification_trick == -1 && tricks.size() <= 3) { // clarification trick not made yet
            if (player != solo_or_marriage_player) { // found marriage partner
                number_of_clarification_trick = tricks.size() - 1;
                set_latest_moment_for_announcements();
                teams_are_known = true;
                players_known_team[player] = 1;
                players_team[player] = 1;
            } else {
                if (tricks.size() == 3) { // no marriage partner found
                    number_of_clarification_trick = 2;
                    set_latest_moment_for_announcements();
                    teams_are_known = true;
                    // all other player's team and known team is already set to 0 in case of a marriage
                }
            }
        }
        tricks.push_back(Trick(game_type, player));
    } else {
        player = next_player(player);
    }
    return player;
}

void GameState::assign_solo_player_to_re_team(int player) {
    players_team[player] = 1;
    players_known_team[player] = 1;
    int others = next_player(player);
    for (int j = 0; j < 3; ++j) { // iterate over the other three players
        players_team[others] = 0;
        players_known_team[others] = 0;
        others = next_player(others);
    }
    teams_are_known = true;
}

void GameState::get_points_and_special_points(int points[4], int &special_points_for_re, bool black[2], int count_re_players) const {
    for (size_t i = 0; i < tricks.size(); ++i) {
        // NOTE: Trick::taken_by() is being calculated twice for all tricks; once during the game and once in this method for counting points and special points
        int trick_taken_by = tricks[i].taken_by();
        if (black[0] && !players_team[trick_taken_by]) // kontra still has no trick but now made one
            black[0] = false;
        if (black[1] && players_team[trick_taken_by]) // re still has no trick but now made one
            black[1] = false;
        int trick_value = tricks[i].get_value_();
        points[trick_taken_by] += trick_value;

        // special points only for regular games and a successful marriage
        if (count_re_players == 2) {
            assert(*game_type == regular || *game_type == marriage);
            int special_points_for_trick_winner = tricks[i].get_special_points_for_trick_winner(
                session_instance, players_team, trick_taken_by, trick_value, i == tricks.size() - 1);
            if (players_team[trick_taken_by])
                special_points_for_re += special_points_for_trick_winner;
            else
                special_points_for_re -= special_points_for_trick_winner;
        }
    }
}

bool GameState::has_team_lost(bool re_team, int points, const bool black[2]) const {
    if ((announcements[re_team] == SCHWARZ && !black[!re_team]) // team announced black and did not get all tricks
        || (announcements[re_team] == N30 && points <= 210) // team announced no 30 and reached less than 211 points
        || (announcements[re_team] == N60 && points <= 180) // team announced no 60 and reached less than 181 points
        || (announcements[re_team] == N90 && points <= 150)) // team announced no 90 and reached less than 151 points
        return true;
    if ((announcements[!re_team] == N90 && points < 90) // team got lass than 90 points against an announcement of no 90
        || (announcements[!re_team] == N60 && points < 60) // team got lass than 60 points against an announcement of no 60
        || (announcements[!re_team] == N30 && points < 30) // team got lass than 30 points against an announcement of no 30
        || (announcements[!re_team] == SCHWARZ && black[re_team]) // team did not make a trick against an announcement of black
        ) {
        return true;
    }
    // special cases follow
    if (announcements[1] == NONE && announcements[0] == REKON) { // re announced nothing, kontra announced kontra
        if (re_team && points < 120) // re team only needs 120 points to win in this case
            return true;
        if (!re_team && points < 121) // kontra team needs 121 points to win in this case
            return true;
    } else if ((announcements[1] == NONE && announcements[0] == NONE)
        || (announcements[1] == REKON && announcements[0] <= REKON)) {
        // (both teams did not announce anything) OR (re announced re and kontra announced nothing or kontra).
        // NOTE: in all other cases where at least one announcement is higher than re or kontra, the above tests decide if the team lost
        if (re_team && points < 121) // "regular" case, re team needs 121 points to win
            return true;
        if (!re_team && points < 120) // "regular" case, kontra team needs 120 points to win
            return true;
    }
    return false;
}

int GameState::get_team_score_points(bool re_team, int points, const bool black[2]) const {
    int score_points = 0;
    if (black[!re_team]) // other team is black
        ++score_points;
    if (points > 210) // other team has less than 30
        ++score_points;
    if (points > 180) // other team has less than 60
        ++score_points;
    if (points > 150) // other team has less than 90
        ++score_points;
    if (announcements[!re_team] >= N90 && points >= 120) // team got at least 120 points against an announcement of no 90
        ++score_points;
    if (announcements[!re_team] >= N60 && points >= 90) // team got at least 90 points against an announcement of no 60
        ++score_points;
    if (announcements[!re_team] >= N30 && points >= 60) // team got at least 60 points against an announcement of no 30
        ++score_points;
    if (announcements[!re_team] >= SCHWARZ && points >= 30) // team got at least 30 points against an announcement of black
        ++score_points;
    return score_points;
}

int GameState::get_winning_team_score_points(bool re_team, int points, const bool black[2]) const {
    int score_points = 1; // game won
    if (announcements[!re_team] >= REKON)
        score_points += 2;
    if (announcements[re_team] >= REKON)
        score_points += 2;

    if (announcements[re_team] >= N90) // team announced no 90
        ++score_points;
    if (announcements[!re_team] >= N90) // other team announced no 90
        ++score_points;

    if (announcements[re_team] >= N60) // team announced no 60
        ++score_points;
    if (announcements[!re_team] >= N60) // other team announced no 60
        ++score_points;

    if (announcements[re_team] >= N30) // team announced no 30
        ++score_points;
    if (announcements[!re_team] >= N30) // other team announced no 30
        ++score_points;

    if (announcements[re_team] == SCHWARZ) // team announced black
        ++score_points;
    if (announcements[!re_team] == SCHWARZ) // other team announced black
        ++score_points;
    return score_points;
}

void GameState::get_score_points(int *players_score_points, int *players_points, int *team_points) const {
    assert(game_finished());

    // calculate the number of re players already here (and not while summing up teams' points), such that get_points_and_special_points only computes special points if the re team consists of two players, i.e. no solo was played
    int count_re_players = 0;
    for (int i = 0; i < 4; ++i)
        if (players_team[i] == 1)
            ++count_re_players;

    int points[4] = { 0, 0, 0, 0 };
    int special_points_for_re = 0;
    bool black[2] = { true, true };
    get_points_and_special_points(points, special_points_for_re, black, count_re_players);
    int points_re = 0;
    int points_kontra = 0;

    if (session_instance)
        cout << "players' playing points:" << endl;
    for (int i = 0; i < 4; ++i) {
        if (session_instance)
            cout << i << ": " << points[i] << endl;
        if (players_points != 0)
            players_points[i] = points[i];
        if (players_team[i] == 1)
            points_re += points[i];
        else
            points_kontra += points[i];
    }
    if (team_points != 0) {
        team_points[0] = points_kontra;
        team_points[1] = points_re;
    }
    assert(points_re + points_kontra == 240);
    if (session_instance)  {
        cout << "re: " << points_re << " kontra: " << points_kontra << endl;
        if (count_re_players == 2) {
            cout << "\nspecial points:" << endl;
            cout << "re: " << special_points_for_re << endl;
            cout << "kontra: " << -special_points_for_re << endl;
        }

        cout << "\nannouncements:" << endl;
        cout << "re: " << announcements[1] << endl;
        cout << "kontra: " << announcements[0] << endl << endl;
    }

    if (black[0])
        assert(points_re == 240);
    bool re_lost = has_team_lost(true, points_re, black);

    if (black[1])
        assert(points_kontra == 240);
    bool kontra_lost = has_team_lost(false, points_kontra, black);

    int score_points_for_re = 0;
    if (re_lost && !kontra_lost) {
        score_points_for_re -= get_team_score_points(false, points_kontra, black);
        score_points_for_re -= get_winning_team_score_points(false, points_kontra, black);
        if (count_re_players == 2) // won against the elders
            --score_points_for_re;
        if (session_instance)
            cout << "kontra has won. score points for re: " << score_points_for_re << endl;
    } else if (!re_lost && kontra_lost) {
        score_points_for_re += get_team_score_points(true, points_re, black);
        score_points_for_re += get_winning_team_score_points(true, points_re, black);
        if (session_instance)
            cout << "re has won. score points for re: " << score_points_for_re << endl;
    } else if (re_lost && kontra_lost) {
        int points_for_re = get_team_score_points(true, points_re, black);
        int points_for_kontra = get_team_score_points(false, points_kontra, black);
        score_points_for_re += points_for_re;
        score_points_for_re -= points_for_kontra;
        if (session_instance)  {
            cout << "nobody won: " << endl;
            cout << "score points for re: " << points_for_re << endl;
            cout << "score points for kontra: " << points_for_kontra << endl;
            cout << "in total: score points for re: " << score_points_for_re << endl;
        }
    } else {
        if (session_instance)
            cout << "both teams did not lose, this cannot happen" << endl;
        assert(false);
    }

    if (count_re_players == 2) {
        score_points_for_re += special_points_for_re;
        if (session_instance)
            cout << "adding up special points. final score points for re: " << score_points_for_re << endl;
    } else { // any kind of solo (including failed marriage or "secret solo" (having two queens of clubs))
        assert(special_points_for_re == 0); // no special points have been calculated!
        if (session_instance)
            cout << "multiplying by 3 for the solo player. final score points for re: " << 3 * score_points_for_re << endl;
    }
    for (int i = 0; i < 4; ++i) {
        if (players_team[i] == 1)
            players_score_points[i] += (count_re_players == 2 ? 1 : 3) * score_points_for_re;
        else
            players_score_points[i] -= score_points_for_re;
    }
}

/*bool _has_team_lost(const announcement_t announcements[2], bool re_team, int points, const bool black[2]) {
    // has been moved back to has_team_lost as the target "test" of the Makefile is not used anymore
}*/
