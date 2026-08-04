#include "perft.h"
#include <iostream>

std::string moveToString(Move move) {
    int from = fromMove(move), to = toMove(move), flags = flagsMove(move);

    std::string s;
    s += char('a' + (from % 8));
    s += char('1' + (from / 8));
    s += char('a' + (to % 8));
    s += char('1' + (to / 8));

    // บิต 3 ติด = โปรโมชั่น, บิต 0-1 บอกว่าเป็นตัวไหน (เรียง N B R Q)
    if (flags >= PROMO_N) s += "nbrq"[flags & 3];
    return s;
}

U64 perft(Position &pos, int depth) {
    if (depth == 0) return 1;

    MoveList list;
    genLegalMoves(pos, list);

    // depth 1 นับจำนวนตาได้เลย ไม่ต้องเดินลงไปอีกชั้น
    if (depth == 1) return (U64)list.count;

    U64 nodes = 0;
    for (int i = 0; i < list.count; ++i) {
        Undo undo;
        doMove(pos, list.moves[i], undo);
        nodes += perft(pos, depth - 1);
        undoMove(pos, list.moves[i], undo);
    }
    return nodes;
}

U64 perftDivide(Position &pos, int depth) {
    MoveList list;
    genLegalMoves(pos, list);

    U64 total = 0;
    for (int i = 0; i < list.count; ++i) {
        Undo undo;
        doMove(pos, list.moves[i], undo);
        U64 nodes = (depth <= 1) ? 1 : perft(pos, depth - 1);
        undoMove(pos, list.moves[i], undo);

        std::cout << moveToString(list.moves[i]) << ": " << nodes << "\n";
        total += nodes;
    }
    std::cout << "\nNodes searched: " << total << "\n";
    return total;
}
