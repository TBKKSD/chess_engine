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

void addPawnMoves(Position &pos, MoveList &moves, int from, int to, bool isCapture) {
    if ((pos.whiteToMove && (to / 8 == 7)) || (!pos.whiteToMove && (to / 8 == 0))) {
        // Promotion
        moves.add(makeMove(from, to, PROMO_N | (isCapture ? CAPTURE : 0)));
        moves.add(makeMove(from, to, PROMO_B | (isCapture ? CAPTURE : 0)));
        moves.add(makeMove(from, to, PROMO_R | (isCapture ? CAPTURE : 0)));
        moves.add(makeMove(from, to, PROMO_Q | (isCapture ? CAPTURE : 0)));
    } else {
        // Normal move
        moves.add(makeMove(from, to, isCapture ? CAPTURE : QUIET));
    }
}

void genPawnMoves(Position &pos, MoveList &moves) {
    int us = pos.whiteToMove ? 0 : 1;
    U64 pawns = pos.pieces[pos.whiteToMove ? WP : BP];
    bool WhiteToMove = pos.whiteToMove;

    while (pawns) {
        int from = popLsb(pawns);
        

        // Normal moves
        U64 Forwardtargets = pos.whiteToMove ? from + 8 : from - 8;
        if(!getBit(pos.occupied[2], Forwardtargets)) {
            addPawnMoves(pos, moves, from, Forwardtargets, false);
        }

        // Double push
        U64 DoublePushtargets = pos.whiteToMove ? from + 16 : from - 16;
        if ((WhiteToMove && (from / 8 == 1) && !getBit(pos.occupied[2], from + 8) && !getBit(pos.occupied[2], DoublePushtargets)) ||
            (!WhiteToMove && (from / 8 == 6) && !getBit(pos.occupied[2], from - 8) && !getBit(pos.occupied[2], DoublePushtargets))) {
            moves.add(makeMove(from, DoublePushtargets, DOUBLE_PUSH));
        }

        // Capture moves
        U64 Capturetargets = PAWN_ATTACKS[us][from] & pos.occupied[1 - us];
        while (Capturetargets) {
            int to = popLsb(Capturetargets);
            addPawnMoves(pos, moves, from, to, true);
        }

        // En passant
        if (pos.epSquare != -1) {
            U64 epTargets = PAWN_ATTACKS[us][from] & (1ULL << pos.epSquare);
            if (epTargets) {
                int to = popLsb(epTargets);
                moves.add(makeMove(from, to));
            }
        }

    }
}


void genSlidingMoves(const Position &pos, MoveList &list) {
    bool white = pos.whiteToMove;
    int us = white ? 0 : 1;
    U64 blockers = pos.occupied[2];
    U64 notOurs  = ~pos.occupied[us];

    struct { int piece; U64 (*attackFn)(int, U64); } tbl[] = {
        { white ? WB : BB, getBishopAttacks },
        { white ? WR : BR, getRookAttacks   },
        { white ? WQ : BQ, getQueenAttacks  },
    };

    for (auto &entry : tbl) {
        U64 bb = pos.pieces[entry.piece];
        while (bb) {
            int from = popLsb(bb);
            U64 targets = entry.attackFn(from, blockers) & notOurs;
            while (targets)
                list.add(makeMove(from, popLsb(targets)));
        }
    }
}