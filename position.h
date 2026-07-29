#pragma once
#include "type.h"
#include <string>

struct Position {
    U64 pieces[12] = {0};
    U64 occupied[3] = {0};
    bool whiteToMove = true;
    int castlingRights = 0;
    int epSquare = -1;
    int halfmoveClock = 0;
};

// bit utilities
inline void setBit(U64 &b, int square) {
    b |= (1ULL << square);
}
inline void clearBit(U64 &b, int square) {
    b &= ~(1ULL << square);
}
inline bool getBit(const U64 &b, int square) {
    return (b >> square) & 1ULL;
}

inline int popCount(U64 b) { return __builtin_popcountll(b); }

inline int popLsb(U64 &b) {
    int sq = __builtin_ctzll(b);
    b &= b - 1;
    return sq;
}

extern const std::string pieceToChar;

int charToPiece(char c);
void parseFEN(Position &pos, const std::string &fen);
int pieceAt(const Position &pos, int square);
void printBoard(const Position &pos);
