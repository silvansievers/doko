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

#include <cassert>
#include <iostream>

using namespace std;

bool default_black[2] = { false, false };

void test(int who_wins, const announcement_t announcements[2], int points_re, const bool black[2] = default_black) {
    bool re_lost = _has_team_lost(announcements, true, points_re, black);
    bool kontra_lost = _has_team_lost(announcements, false, 240 - points_re, black);
    switch (who_wins) {
        case 1: // re should win
            assert(kontra_lost);
            assert(!re_lost);
            break;
        case 0: // kontra should win
            assert(re_lost);
            assert(!kontra_lost);
            break;
        case -1: // nobody should win
            assert(re_lost);
            assert(kontra_lost);
            break;
        default:
            assert(false);
    }
}

int main() {
    announcement_t announcements[2] = { NONE, NONE };
    test(0, announcements, 120);
    announcements[0] = REKON;
    test(1, announcements, 120);
    announcements[0] = N90;
    announcements[1] = N90;
    test(-1, announcements, 150);
    test(-1, announcements, 90);
    test(0, announcements, 89);
    test(1, announcements, 151);
    announcements[0] = NONE;
    test(0, announcements, 150);

    announcements[1] = SCHWARZ;
    bool black[2] = { false, false };
    test(0, announcements, 240, black);
    black[0] = true;
    test(1, announcements, 240, black);
    return 0;
}
