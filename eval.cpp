#include "eval.h"

constexpr int PHASE_WEIGHT[6] = {0, 1, 1, 2, 4, 0}; // P N B R Q K

constexpr int TOTAL_PHASE = 24; // 16+4+4+2+2

int evaluate(const Position &pos) {
    int mg = 0, eg = 0, phase = 0;

    for (int p = 0; p < 12; ++p) {
        bool white = (p < 6);
        int type = p % 6;
        U64 bb = pos.pieces[p];

        while (bb) {
            int sq = popLsb(bb);
            int idx = white ? flipSquare(sq) : sq;
            int sign = white ? 1 : -1;

            mg += sign * (PIECE_VALUE[MG][type] + PST_MG[type][idx]);
            eg += sign * (PIECE_VALUE[EG][type] + PST_EG[type][idx]);
            phase += PHASE_WEIGHT[type];
        }
    }

    if(phase > TOTAL_PHASE) phase = TOTAL_PHASE;
    int score = (mg * phase + eg * (TOTAL_PHASE - phase)) / TOTAL_PHASE;

    return pos.whiteToMove ? score : -score;
}