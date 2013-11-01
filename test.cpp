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