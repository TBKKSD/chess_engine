#include "attacks.h"

U64 PAWN_ATTACKS[2][64];
U64 KNIGHT_ATTACKS[64];
U64 KING_ATTACKS[64];

void initAttacksTables() {
    const int kn_moves[8][2] = {
        {1, 2}, {2, 1}, {2, -1}, {1, -2},
        {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}
    };
    const int king_moves[8][2] = {
        {1, 0}, {1, 1}, {0, 1}, {-1, 1},
        {-1, 0}, {-1, -1}, {0, -1}, {1, -1}
    };

    for (int sq = 0; sq < 64; sq++) {
        int rank = sq / 8;
        int file = sq % 8;

        U64 kn_attacks = 0;
        U64 king_attacks = 0;
        for (int i=0; i<8; i++) {
            //Knight attacks
            int knr = rank + kn_moves[i][0];
            int knf = file + kn_moves[i][1];
            if (knr >= 0 && knr < 8 && knf >= 0 && knf < 8) {
                setBit(kn_attacks, knr * 8 + knf);
            }
            //King attacks
            int kr = rank + king_moves[i][0];
            int kf = file + king_moves[i][1];
            if (kr >= 0 && kr < 8 && kf >= 0 && kf < 8) {
                setBit(king_attacks, kr * 8 + kf);
            }
        }
        KNIGHT_ATTACKS[sq] = kn_attacks;
        KING_ATTACKS[sq] = king_attacks;
    }
}