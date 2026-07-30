#include "movegen.h"

void genKnightMoves(Position &pos, MoveList &moves) {
    int us = pos.whiteToMove ? 0 : 1;
    U64 knights = pos.pieces[us * 6 + 1]; // WN or BN

    while (knights) {
        int from = popLsb(knights);
        U64 targets = KNIGHT_ATTACKS[from] & ~pos.occupied[us];

        while (targets) {
            int to = popLsb(targets);
            moves.add(makeMove(from, to));
        }
    }
}

void genKingMoves(Position &pos, MoveList &moves) {
    int us = pos.whiteToMove ? 0 : 1;
    U64 king = pos.pieces[us * 6 + 5]; // WK or BK

    if (king) {
        int from = popLsb(king);
        U64 targets = KING_ATTACKS[from] & ~pos.occupied[us];

        while (targets) {
            int to = popLsb(targets);
            moves.add(makeMove(from, to));
        }
    }
}