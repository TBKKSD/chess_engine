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
        U64 white_pawn_attacks = 0;
        U64 black_pawn_attacks = 0;
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

        const int white_pawn_moves[2][2] = {
        {1, 1}, {1, -1}
        };
        const int black_pawn_moves[2][2] = {
            {-1, 1}, {-1, -1}
        };
        for (int i = 0; i < 2; i++) {
            //White pawn attacks
            int wpr = rank + white_pawn_moves[i][0];
            int wpf = file + white_pawn_moves[i][1];
            if (wpr >= 0 && wpr < 8 && wpf >= 0 && wpf < 8) {
                setBit(white_pawn_attacks, wpr * 8 + wpf);
            }
            //Black pawn attacks
            int bpr = rank + black_pawn_moves[i][0];
            int bpf = file + black_pawn_moves[i][1];
            if (bpr >= 0 && bpr < 8 && bpf >= 0 && bpf < 8) {
                setBit(black_pawn_attacks, bpr * 8 + bpf);
            }
        }
        PAWN_ATTACKS[0][sq] = white_pawn_attacks;
        PAWN_ATTACKS[1][sq] = black_pawn_attacks;
    }
}

static const int ROOK_DIR[4][2]   = {{1,0},{-1,0},{0,1},{0,-1}};
static const int BISHOP_DIR[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};

// เดินไล่ทีละช่องตามทิศที่กำหนด
static U64 slidingAttacks(int sq, U64 blockers, const int dir[4][2]) {
    U64 attacks = 0;
    int r = sq / 8, f = sq % 8;

    for (int d = 0; d < 4; ++d) {
        int nr = r, nf = f;
        while (true) {
            nr += dir[d][0];
            nf += dir[d][1];
            if (nr < 0 || nr > 7 || nf < 0 || nf > 7) break;   // หลุดกระดาน

            int to = nr * 8 + nf;
            setBit(attacks, to);                 // ใส่ก่อนเสมอ
            if (getBit(blockers, to)) break;     // มีหมาก → หยุดหลังใส่แล้ว
        }
    }
    return attacks;
}
U64 getRookAttacks(int sq, U64 blockers)   { return slidingAttacks(sq, blockers, ROOK_DIR); }
U64 getBishopAttacks(int sq, U64 blockers) { return slidingAttacks(sq, blockers, BISHOP_DIR); }
U64 getQueenAttacks(int sq, U64 blockers)  { return getRookAttacks(sq, blockers) | getBishopAttacks(sq, blockers); }