#include <cstdint>
#include <string>
#include <iostream>

using U64 = uint64_t;

enum Piece {WP, WN, WB, WR, WQ, WK,
            BP, BN, BB, BR, BQ, BK};

enum Square {A1, B1, C1, D1, E1, F1, G1, H1,
            A2, B2, C2, D2, E2, F2, G2, H2,
            A3, B3, C3, D3, E3, F3, G3, H3,
            A4, B4, C4, D4, E4, F4, G4, H4,
            A5, B5, C5, D5, E5, F5, G5, H5,
            A6, B6, C6, D6, E6, F6, G6, H6,
            A7, B7, C7, D7, E7, F7, G7, H7,
            A8, B8, C8, D8, E8, F8, G8, H8};

struct Position {
    U64 pieces[12] = {0}; 
    U64 occupied[3] = {0};
    bool whiteToMove = true;
    int castlingRights = 0;
    int enPassantSquare = -1;
    int halfmoveClock = 0;
};

inline void setBit(U64 &b, Square square) {
    b |= (1ULL << square);
}
inline void clearBit(U64 &b, Square square) {
    b &= ~(1ULL << square);
}
inline bool getBit(const U64 &b, Square square) {
    return (b >> square) & 1ULL;
}

inline int popCount(U64 b) { return __builtin_popcountll(b);  }

inline int popLsb(U64 &b) {
    int sq = __builtin_ctzll(b);
    b &= b - 1;
    return sq;
}
