#pragma once

#include <cstdint>

using U64 = uint64_t;
using Move = uint16_t;

enum Square {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8
};

enum Piece {WP, WN, WB, WR, WQ, WK,
            BP, BN, BB, BR, BQ, BK, NO_PIECE};

// move encoding: bits 0-5 = from, 6-11 = to, 12-15 = flags
inline Move makeMove(int from, int to, int flags = 0) {
    return from | (to << 6) | (flags << 12);
}

inline int fromMove(Move move) { return move & 0x3F; }
inline int toMove(Move move) { return (move >> 6) & 0x3F; }
inline int flagsMove(Move move) { return (move >> 12) & 0xF; }

struct MoveList {
    Move moves[256];
    int count = 0;
    void add(Move move) { moves[count++] = move; }
};

constexpr U64 FILE_A = 0x0101010101010101ULL;
constexpr U64 FILE_H = 0x8080808080808080ULL;
constexpr U64 RANK_1 = 0x00000000000000FFULL;
constexpr U64 RANK_3 = 0x0000000000FF0000ULL;
constexpr U64 RANK_6 = 0x0000FF0000000000ULL;
constexpr U64 RANK_8 = 0xFF00000000000000ULL;

enum MoveFlag {
    QUIET = 0, DOUBLE_PUSH = 1, KING_CASTLE = 2, QUEEN_CASTLE = 3,
    CAPTURE = 4, EP_CAPTURE = 5,
    PROMO_N = 8,  PROMO_B = 9,  PROMO_R = 10, PROMO_Q = 11,
    PROMO_N_CAP = 12, PROMO_B_CAP = 13, PROMO_R_CAP = 14, PROMO_Q_CAP = 15,
};
