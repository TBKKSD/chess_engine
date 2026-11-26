#include "tt.h"

std::vector<TTEntry> ttTable;

void ttInit(int sizeMB) {
    ttTable.resize(((size_t)sizeMB * 1024 * 1024) / sizeof(TTEntry));
}

void ttClear() {
    std::fill(ttTable.begin(), ttTable.end(), TTEntry{});
}

bool ttProbe(U64 key, TTEntry &out) {
    // Implementation for probing the transposition table
    const TTEntry &e = ttTable[key & (ttTable.size() - 1)];
    if (e.key != key) return false;
    out = e;
    return true;
}

void ttStore(U64 key, int depth, int score, int flag, Move bestMove) {
    // Implementation for storing entries in the transposition table
    TTEntry entry;
    entry.key = key;
    entry.depth = depth;
    entry.score = score;
    entry.flag = flag;
    entry.bestMove = bestMove;
    ttTable[key & (ttTable.size() - 1)] = entry;
}